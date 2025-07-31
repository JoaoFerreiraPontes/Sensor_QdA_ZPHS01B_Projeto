#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "zphs01b.h"
#include "bt.h"

// --- DEFINIÇÕES GERAIS ---
// Pinos para a comunicação UART com o sensor, vindos da configuração do projeto (menuconfig)
#define UART_TXD_PIN (CONFIG_EXAMPLE_UART_TXD)
#define UART_RXD_PIN (CONFIG_EXAMPLE_UART_RXD)
#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)
// Configurações da porta UART
#define UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)
// Tamanho do buffer para receber dados da UART
#define BUF_SIZE           (1024)
// Tamanho esperado da resposta do sensor (em bytes), conforme o datasheet
#define RESPONSE_LENGTH    (26)
// Tamanho máximo da mensagem formatada para envio via Bluetooth
#define RESULT_MESSAGE_SIZE (500)

// --- VARIÁVEIS GLOBAIS DO MÓDULO ---
// Handle (identificador) da tarefa do sensor, para podermos pará-la e iniciá-la
static TaskHandle_t zphs01b_task_handle = NULL;
// Tag para os logs deste arquivo, facilita a depuração
static const char *TAG_UART = "ZPHS01B_UART";
// Comando exato em bytes para solicitar os dados do sensor ZPHS01B
static const uint8_t ZPHS01B_DATA_REQUEST[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
// Calcula o tamanho do comando de solicitação
static const uint8_t ZPHS01B_DATA_REQUEST_LEN = sizeof(ZPHS01B_DATA_REQUEST)/sizeof(uint8_t);


// --- ESTRUTURAS DE DADOS ---
// Enum para classificar os níveis de poluição (Baixo, Médio, Alto, Erro)
typedef enum lvl {LO = 0, ME = 1, HI = 2, ER = 3} lvl_e;
// Array de strings para converter o enum em texto legível
const char *lvls[] = {[LO] = "Low", [ME] = "Med.", [HI] = "High", [ER] = "error"};
// Estrutura principal que armazena todos os dados lidos e processados do sensor
static struct air_data {
    uint16_t pm1_0;    lvl_e pm1_0_lvl;
    uint16_t pm2_5;    lvl_e pm2_5_lvl;
    uint16_t pm10;     lvl_e pm10_lvl;
    uint16_t co2;      lvl_e co2_lvl;
    uint8_t  voc;      lvl_e voc_lvl;
    uint16_t ch2o;     lvl_e ch2o_lvl;
    double co;         lvl_e co_lvl;
    uint16_t o3;       lvl_e o3_lvl;
    uint16_t no2;      lvl_e no2_lvl;
    double temp;
    uint16_t humidity; lvl_e humidity_lvl;
} air_data_processed;

// --- PROTÓTIPOS DE FUNÇÕES ESTÁTICAS ---
// (Declarações antecipadas das funções usadas apenas neste arquivo)
static int construct_output_message(const struct air_data *d, char *output_message);
static void process_response(const uint8_t *response, const int response_len, struct air_data *output);
static uint8_t check_response(uint8_t *response, int response_length);
static void reset_buffers_and_counter(uint8_t *response, int *response_len, char *output_message);
static lvl_e get_pm1_0_lvl(uint16_t pm1_0);
static lvl_e get_pm2_5_lvl(uint16_t pm2_5);
static lvl_e get_pm10_lvl(uint16_t pm10);
static lvl_e get_co2_lvl(uint16_t co2);
static lvl_e get_voc_lvl(uint8_t voc);
static lvl_e get_ch2o_lvl(uint16_t ch2o);
static lvl_e get_co_lvl(double co);
static lvl_e get_o3_lvl(uint16_t o3);
static lvl_e get_no2_lvl(uint16_t no2);
static lvl_e get_humidity_lvl(uint16_t rh);
static void zphs01b_task(void *arg);


/**
 * @brief Tarefa principal que roda em loop para ler os dados do sensor.
 * * @param arg Ponteiro para o valor do intervalo de leitura em milissegundos.
 */
static void zphs01b_task(void *arg) {
    // Converte o argumento recebido para o intervalo de tempo
    uint32_t read_data_pause_ms = (uint32_t)arg;
    ESP_LOGI(TAG_UART, "Task iniciada com intervalo de %lu ms.", read_data_pause_ms);

    // Aloca memória para os buffers de dados
    uint8_t *data = (uint8_t *) calloc(BUF_SIZE, sizeof(uint8_t));
    int len = 0;
    char *output_message = (char *) calloc(RESULT_MESSAGE_SIZE, sizeof(char));
    if (data == NULL || output_message == NULL) {
        ESP_LOGE(TAG_UART, "Erro de alocação de memória.");
        vTaskDelete(NULL); // Encerra a tarefa se não houver memória
        return;
    }

    // Loop infinito da tarefa
    while (1) {
        // Envia o comando para o sensor pedindo novos dados
        uart_write_bytes(UART_PORT_NUM, ZPHS01B_DATA_REQUEST, ZPHS01B_DATA_REQUEST_LEN);
        // Lê a resposta do sensor com um timeout de 1 segundo
        len = uart_read_bytes(UART_PORT_NUM, data, (BUF_SIZE - 1), pdMS_TO_TICKS(1000));
        
        // Verifica se a resposta do sensor é válida (checksum)
        if (check_response(data, len)) {
            ESP_LOGW(TAG_UART, "Resposta do sensor invalida.");
        } else {
            // Se for válida, processa os bytes e converte para valores legíveis
            process_response(data, len, &air_data_processed);
            // Formata a mensagem final para ser enviada
            if (construct_output_message(&air_data_processed, output_message)) {
                // Envia a mensagem via Bluetooth
                send_message(output_message);
            }
        }
        // Limpa os buffers para a próxima leitura
        reset_buffers_and_counter(data, &len, output_message);
        // Pausa a tarefa pelo intervalo de tempo definido pelo usuário
        vTaskDelay(pdMS_TO_TICKS(read_data_pause_ms));
    }
}

/**
 * @brief Inicializa o driver da UART para comunicação com o sensor.
 * Esta função deve ser chamada apenas uma vez no início do programa.
 */
void zphs01b_uart_init(void) {
    // Estrutura de configuração da UART
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    // Instala e configura o driver UART com os pinos definidos
    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD_PIN, UART_RXD_PIN, UART_RTS, UART_CTS));
    ESP_LOGI(TAG_UART, "Driver UART do sensor ZPHS01B inicializado.");
}

/**
 * @brief Funções que classificam os valores lidos em níveis (Baixo, Médio, Alto).
 * Os valores de referência são baseados em padrões de qualidade do ar.
 */
static lvl_e get_pm1_0_lvl(uint16_t pm1_0) {
    if (pm1_0 <= 10) return LO;
    else if (pm1_0 <= 25) return ME;
    else if (pm1_0 <= 1000) return HI;
    else return ER;
}

static lvl_e get_pm2_5_lvl(uint16_t pm2_5) {
    if (pm2_5 < 14) return LO;
    else if (pm2_5 < 25) return ME;
    else if (pm2_5 <= 1000) return HI;
    else return ER;
}

static lvl_e get_pm10_lvl(uint16_t pm10) {
    if (pm10 < 20) return LO;
    else if (pm10 < 50) return ME;
    else if (pm10 <= 1000) return HI;
    else return ER;
}

static lvl_e get_co2_lvl(uint16_t co2) {
    if (co2 <= 700) return LO;
    else if (co2 <= 1200) return ME;
    else if (co2 <= 5000) return HI;
    else return ER;
}

static lvl_e get_voc_lvl(uint8_t voc) {
    if (voc == 0) return LO;
    else if (voc == 1) return ME;
    else if (voc == 2 || voc == 3) return HI;
    else return ER;
}

static lvl_e get_ch2o_lvl(uint16_t ch2o) {
    if (ch2o < 10) return LO;
    else if (ch2o < 36) return ME;
    else if (ch2o <= 6250) return HI;
    else return ER;
}

static lvl_e get_co_lvl(double co) {
    if (co >= 0 && co < 9) return LO;
    else if (co < 25) return ME;
    else if (co <= 500) return HI;
    else return ER;
}

static lvl_e get_o3_lvl(uint16_t o3) {
    if (o3 < 21) return LO;
    else if (o3 < 55) return ME;
    else if (o3 <= 10000) return HI;
    else return ER;
}

static lvl_e get_no2_lvl(uint16_t no2) {
    if (no2 < 51) return LO;
    else if (no2 < 100) return ME;
    else if (no2 <= 10000) return HI;
    else return ER;
}

static lvl_e get_humidity_lvl(uint16_t rh) {
    if (rh < 30) return LO;
    else if (rh <= 68) return ME;
    else if (rh <= 100) return HI;
    else return ER;
}

/**
 * @brief Formata a string final com todos os dados para ser exibida.
 */
static int construct_output_message(const struct air_data *d, char *output_message) {
    if (output_message == NULL) return 0;
    // Usa sprintf para montar a mensagem com os níveis e os valores numéricos
    int rv = sprintf(output_message,
          "\n\npm1.0 %s, pm2.5 %s, pm10 %s, CO2 %s, TVOC %s, CH2O %s, CO %s, O3 %s, NO2 %s, RH %s;\n"
          "pm1.0 %d ug/m3, pm2.5 %d ug/m3, pm10 %d ug/m3, CO2 %d ppm, TVOC %d lvl, CH2O %d ug/m3, CO %.1f ppm, O3 %d ppb, NO2 %d ppb, %.1f *C, %d%% RH;\n"
          "\n>> Caso queira alterar a frequencia de recebimento de dados, aperte 'X'.\n",
           lvls[d->pm1_0_lvl], lvls[d->pm2_5_lvl], lvls[d->pm10_lvl], lvls[d->co2_lvl], lvls[d->voc_lvl], lvls[d->ch2o_lvl], lvls[d->co_lvl], lvls[d->o3_lvl], lvls[d->no2_lvl], lvls[d->humidity_lvl],
            d->pm1_0, d->pm2_5, d->pm10, d->co2, d->voc, d->ch2o, d->co, d->o3, d->no2, d->temp, d->humidity);

        if (rv <= 0) { output_message[0] = '\0'; return 0; }

        ESP_LOGI("OUTPUT_MSG", "%s", output_message);

         return 1;

}
/**
 * @brief Processa o array de bytes recebido do sensor e preenche a struct air_data.
 * A ordem e o cálculo dos bytes seguem o datasheet do ZPHS01B.
 */
static void process_response(const uint8_t *response, const int response_len, struct air_data *output) {
    if (response_len != RESPONSE_LENGTH) return;
    // Exemplo: PM1.0 é formado por 2 bytes. Multiplica-se o primeiro por 256 e soma com o segundo.
    output->pm1_0 = response[2]*256 + response[3]; output->pm1_0_lvl = get_pm1_0_lvl(output->pm1_0);
    output->pm2_5 = response[4]*256 + response[5]; output->pm2_5_lvl = get_pm2_5_lvl(output->pm2_5);
    output->pm10 = response[6]*256 + response[7]; output->pm10_lvl = get_pm10_lvl(output->pm10);
    output->co2 = response[8]*256 + response[9]; output->co2_lvl = get_co2_lvl(output->co2);
    output->voc = response[10]; output->voc_lvl = get_voc_lvl(output->voc);
    output->temp = (((response[11]*256) + response[12]) - 450) * 0.1;
    output->humidity = response[13]*256 + response[14]; output->humidity_lvl = get_humidity_lvl(output->humidity);
    output->ch2o = response[15]*256 + response[16]; output->ch2o_lvl = get_ch2o_lvl(output->ch2o);
    output->co = (response[17]*256 + response[18]) * 0.1; output->co_lvl = get_co_lvl(output->co);
    output->o3 = (response[19]*256 + response[20]) * 10; output->o3_lvl = get_o3_lvl(output->o3);
    output->no2 = (response[21]*256 + response[22]) * 10; output->no2_lvl = get_no2_lvl(output->no2);
}

/**
 * @brief Valida a resposta do sensor calculando e comparando o checksum.
 */
static uint8_t check_response(uint8_t *response, int response_length) {
    uint8_t checksum = 0;
    if (response_length != RESPONSE_LENGTH) { return 1; }
    // Soma todos os bytes da resposta (exceto o primeiro e o último)
    for (int i = 1; i < 25; i++) { checksum += response[i]; }
    // Aplica a fórmula do datasheet: invert + 1
    checksum = ~checksum + 1;
    // Compara com o byte de checksum enviado pelo sensor (o último byte)
    if (checksum != response[25]) { return 1; } // Retorna 1 se houver erro
    return 0; // Retorna 0 se estiver ok
}

/**
 * @brief Limpa os buffers de dados após cada ciclo de leitura.
 */
static void reset_buffers_and_counter(uint8_t *response, int *response_len, char *output_message) {
    if (*response_len > 0) { memset(response, 0, *response_len); }
    memset(output_message, 0, RESULT_MESSAGE_SIZE);
    *response_len = 0;
}


// --- FUNÇÕES PÚBLICAS (Chamadas por outros arquivos, como o main.c) ---

/**
 * @brief Cria e inicia a tarefa do sensor com um intervalo de tempo específico.
 * Se uma tarefa antiga existir, ela é deletada primeiro.
 * @param delay_ms Intervalo de tempo entre as leituras.
 */
void init_and_run_zphs01b(uint32_t delay_ms) {
    if (zphs01b_task_handle != NULL) {
        vTaskDelete(zphs01b_task_handle);
        zphs01b_task_handle = NULL;
    }
    // Cria a tarefa e armazena seu handle (identificador)
    xTaskCreate(zphs01b_task, "zphs01b_task", TASK_STACK_SIZE, (void*)delay_ms, 10, &zphs01b_task_handle);
}

/**
 * @brief Para e deleta a tarefa do sensor.
 */
void stop_zphs01b_task(void) {
    if (zphs01b_task_handle != NULL) {
        vTaskDelete(zphs01b_task_handle);
        zphs01b_task_handle = NULL;
    }
}