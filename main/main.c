#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "bt.h"
#include "zphs01b.h"

#define DEFAULT_REFRESH_RATE 5000
#define MIN_REFRESH_RATE     1500
#define MAX_REFRESH_RATE     29000
#define INPUT_TIMEOUT_S      180
#define STARTUP_DELAY_S      10

static const char *TAG_MAIN = "APP_MAIN";

// Função para ler a entrada do usuário com eco e timeout
static int read_user_input_with_echo(char *buffer, int max_len, int timeout_ms) {
    char c;
    int len = 0;
    TickType_t start_ticks = xTaskGetTickCount();
    memset(buffer, 0, max_len);
    while ((xTaskGetTickCount() - start_ticks) * portTICK_PERIOD_MS < timeout_ms) {
        if (uart_read_bytes(UART_NUM_0, (uint8_t *)&c, 1, pdMS_TO_TICKS(20)) > 0) {
            if (c == '\r' || c == '\n') { printf("\n"); fflush(stdout); return len; }
            if ((c == '\b' || c == 127) && len > 0) { len--; printf("\b \b"); fflush(stdout); }
            else if (c >= '0' && c <= '9' && len < max_len - 1) { buffer[len++] = c; printf("%c", c); fflush(stdout); }
        }
    }
    printf("\n"); fflush(stdout); return len;
}

// Função para encapsular a lógica de obter a frequência do usuário
uint32_t get_frequency_from_user(void) {
    uint32_t refresh_rate = DEFAULT_REFRESH_RATE;
    char input_buffer[16];
    
    printf("------------------------------------------------------------------\n");
    printf("Digite APENAS NUMEROS para definir o intervalo em milissegundos (ms).\n");
    printf("Valores aceitos: de %dms a %dms.\n", MIN_REFRESH_RATE, MAX_REFRESH_RATE);
    printf("Pressione Enter para confirmar ou aguarde %d segundos para usar o tempo padrao (%dms).\n", INPUT_TIMEOUT_S, DEFAULT_REFRESH_RATE);
    printf("Intervalo: ");
    fflush(stdout); 

    int length = read_user_input_with_echo(input_buffer, sizeof(input_buffer), INPUT_TIMEOUT_S * 1000);

    if (length > 0) {
        int input_val = atoi(input_buffer);
        if (input_val >= MIN_REFRESH_RATE && input_val <= MAX_REFRESH_RATE) {
            refresh_rate = (uint32_t)input_val;
            ESP_LOGI(TAG_MAIN, "Valor valido recebido.");
        } else {
            ESP_LOGW(TAG_MAIN, "Valor '%s' e invalido ou fora do intervalo. Usando o valor padrao.", input_buffer);
            refresh_rate = DEFAULT_REFRESH_RATE;
        }
    } else {
        ESP_LOGI(TAG_MAIN, "Nenhum dado inserido (timeout). Usando o valor padrao.");
    }

    printf("\nVoce definiu o tempo de recebimento em %lu ms.\n", refresh_rate);
    printf("------------------------------------------------------------------\n\n");
    vTaskDelay(pdMS_TO_TICKS(1000)); // Pequena pausa antes de iniciar o sensor
    return refresh_rate;
}


void app_main(void)
{
    // Inicializa a UART do console para entrada do usuário
    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);

    printf("Iniciando em %d segundos...\n", STARTUP_DELAY_S);
    vTaskDelay(pdMS_TO_TICKS(STARTUP_DELAY_S * 1000));

    // Mensagem de boas-vindas detalhada 
    printf("\n\n--- Sensor de Qualidade do Ar com ZPHS01B ---\n");
    printf("Este projeto utiliza o sensor ZPHS01B, um modulo multi-parametros que mede\n");
    printf("particulas (PM1.0, PM2.5, PM10), gases (CO2, CO, O3, NO2), Compostos Organicos\n");
    printf("Volateis (VOC), formaldeido (CH2O), temperatura e umidade.\n");
    printf("Autor do Projeto: Joao Abrantes Pontes\n");
    printf("Repositorio do Projeto: https://github.com/JoaoFerreiraPontes/Sensor_QdA_ZPHS01B_Projeto\n");
    printf("\nAVISO IMPORTANTE:\n");
    printf("O sensor requer um periodo de calibracao de 10 minutos para que as medicoes\n");
    printf("se estabilizem. Recomenda-se utilizar os dados para tratamento real somente\n");
    printf("apos este periodo.\n\n");

    // Inicializa o bluetooth e a UART do sensor 
    zphs01b_uart_init(); // <-- chamada da nova função de inicializaçã do sensor

    // Loop principal do programa
    while (1) {
        uint32_t current_frequency = get_frequency_from_user();
        init_and_run_zphs01b(current_frequency);

        // Loop de monitoramento: se mantém enquanto o sensor roda
        while (1) {
            char c;
            // Verifica se o usuário digitou algo, sem bloquear
            if (uart_read_bytes(UART_NUM_0, (uint8_t *)&c, 1, pdMS_TO_TICKS(100)) > 0) {
                if (c == 'X' || c == 'x') {
                    printf("\n>> Solicitacao para alterar frequencia recebida! <<\n");
                    stop_zphs01b_task(); // Para a tarefa do sensor
                    vTaskDelay(pdMS_TO_TICKS(500)); // Pequena pausa
                    break; // Sai do loop de monitoramento para voltar ao menu
                }
            }
        }
    }
}