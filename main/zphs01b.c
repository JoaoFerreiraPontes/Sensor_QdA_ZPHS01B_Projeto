
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

/**
 * ESP-IDF v5.0.7
 * - Port: configuração UART, baudrate 9600
 * - Atribuição de pinos: veja as definições abaixo (veja Kconfig)
 */

#define UART_TXD_PIN (CONFIG_EXAMPLE_UART_TXD)
#define UART_RXD_PIN (CONFIG_EXAMPLE_UART_RXD)
#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

#define BUF_SIZE            (1024)
#define RESPONSE_LENGTH     (26)
#define READ_DATA_PAUSE_MS  (29000)   //pause before next read in milliseconds
#define RESULT_MESSAGE_SIZE (500)

static const char *TAG = "UART";
static const uint8_t ZPHS01B_DATA_REQUEST[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
static const uint8_t ZPHS01B_DATA_REQUEST_LEN = sizeof(ZPHS01B_DATA_REQUEST)/sizeof(uint8_t);

typedef enum lvl {LO = 0, ME = 1, HI = 2, ER = 3} lvl_e;
const char *lvls[] = {[LO] = "Low", [ME] = "Med.", [HI] = "High", [ER] = "error"};  //usage: printf("%s", lvlnames[lvl_e value]);

static struct air_data {
    uint16_t pm1_0;
    lvl_e pm1_0_lvl;
    uint16_t pm2_5;
    lvl_e pm2_5_lvl;
    uint16_t pm10;
    lvl_e pm10_lvl;
    uint16_t co2;
    lvl_e co2_lvl;
    uint8_t  voc;
    lvl_e voc_lvl;
    uint16_t ch2o;
    lvl_e ch2o_lvl;
    double co;
    lvl_e co_lvl;
    uint16_t o3;
    lvl_e o3_lvl;
    uint16_t no2;
    lvl_e no2_lvl;
    double temp;
    uint16_t humidity;
    lvl_e humidity_lvl;
} air_data_processed;

static uint8_t check_response(uint8_t *response, int response_length);
static void reset_buffers_and_counter(uint8_t *response, int *response_len, char *output_message);
static void process_response(const uint8_t *response, const int response_len, struct air_data *output);
static int construct_output_message(const struct air_data *data, char *output_message);


static void zphs01b_task(void *arg) {
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD_PIN, UART_RXD_PIN, UART_RTS, UART_CTS));

    // Configure a buffer for the incoming data
    uint8_t *data = (uint8_t *) calloc(BUF_SIZE, sizeof(uint8_t));
    int len = 0;
    char *output_message = (char *) calloc(RESULT_MESSAGE_SIZE, sizeof(char));
    if (data == NULL || output_message == NULL) {
        ESP_LOGE("", "Error during allocation of memory for UART data and output message.");
        return;
    }

    while (1) {

        // Write data to the ZPHS01B module
        uart_write_bytes(UART_PORT_NUM, ZPHS01B_DATA_REQUEST, ZPHS01B_DATA_REQUEST_LEN);

        // Read data from the ZPHS01B module
        len = uart_read_bytes(UART_PORT_NUM, data, (BUF_SIZE - 1), pdMS_TO_TICKS(200));

        if (check_response(data, len)) {
            reset_buffers_and_counter(data, &len, output_message);
            ESP_LOGI(TAG, "error in response data checksum");
            continue;
        } else {
            ESP_LOGI(TAG, "response check is passed");
        }

        process_response(data, len, &air_data_processed);
        if (construct_output_message(&air_data_processed, output_message)) {
            send_message(output_message);
        }

        reset_buffers_and_counter(data, &len, output_message);

        vTaskDelay(pdMS_TO_TICKS(READ_DATA_PAUSE_MS));
    }
}

static lvl_e get_pm1_0_lvl(uint16_t pm1_0) {
    if (pm1_0 >= 0 && pm1_0 <= 10) {
        return LO;
    } else if (pm1_0 > 10 && pm1_0 <= 25) {
        return ME;
    } else if (pm1_0 > 25 && pm1_0 <= 1000) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_pm2_5_lvl(uint16_t pm2_5) {
    if (pm2_5 >= 0 && pm2_5 < 14) {
        return LO;
    } else if (pm2_5 >= 14 && pm2_5 < 25 ) {
        return ME;
    } else if (pm2_5 >= 25 && pm2_5 <= 1000) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_pm10_lvl(uint16_t pm10) {
    if (pm10 >= 0 && pm10 < 20) {
        return LO;
    } else if (pm10 >= 20 && pm10 < 50) {
        return ME;
    } else if (pm10 >= 50 && pm10 <= 1000) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_co2_lvl(uint16_t co2) {
    if (co2 >= 0 && co2 <= 700) {
        return LO;
    } else if (co2 > 700 && co2 <= 1200) {
        return ME;
    } else if (co2 > 1200 && co2 <= 5000) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_voc_lvl(uint8_t voc) {
    if (voc == 0) {
        return LO;
    } else if (voc == 1) {
        return ME;
    } else if (voc == 2 || voc == 3) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_ch2o_lvl(uint16_t ch2o) {
    if (ch2o >= 0 && ch2o < 10) {
        return LO;
    } else if (ch2o >= 10 && ch2o < 36) {
        return ME;
    } else if (ch2o >= 36 && ch2o <= 6250) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_co_lvl(double co) {
    if (co >= 0 && co < 9) {
        return LO;
    } else if (co >= 9 && co < 25) {
        return ME;
    } else if (co >= 25 && co <= 500) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_o3_lvl(uint16_t o3) {
    if (o3 >= 0 && o3 < 21) {
        return LO;
    } else if (o3 >= 21 && o3 < 55) {
        return ME;
    } else if (o3 >= 55 && o3 <= 10000) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_no2_lvl(uint16_t no2) {
    if (no2 >= 0 && no2 < 51) {
        return LO;
    } else if (no2 >= 51 && no2 < 100) {
        return ME;
    } else if (no2 >= 100 && no2 <= 10000) {
        return HI;
    } else {
        return ER;
    }
}

static lvl_e get_humidity_lvl(uint16_t rh) {
    if (rh >= 0 && rh < 30) {
        return LO;
    } else if (rh >= 30 && rh <= 68) {
        return ME;
    } else if (rh > 68 && rh <= 100) {
        return HI;
    } else {
        return ER;
    }
}


/*
    The output parameter has a null-terminated string with human readable measurement results.
*/
static void process_response(const uint8_t *response, const int response_len, struct air_data *output) {
    if (response_len != RESPONSE_LENGTH) {
        ESP_LOGI(TAG, "wrong reponse length in data process_response()");
        return;
    }

    output->pm1_0 = response[2]*256 + response[3]; //0..1000 ug/m3
    output->pm1_0_lvl = get_pm1_0_lvl(output->pm1_0);

    output->pm2_5 = response[4]*256 + response[5]; //0..1000 ug/m3
    output->pm2_5_lvl = get_pm2_5_lvl(output->pm2_5);

    output->pm10 = response[6]*256 + response[7];  //0..1000 ug/m3
    output->pm10_lvl = get_pm10_lvl(output->pm10);

    output->co2 = response[8]*256 + response[9];   //0..5000 ppm
    output->co2_lvl = get_co2_lvl(output->co2);

    output->voc = response[10];                    //0..3 grades
    output->voc_lvl = get_voc_lvl(output->voc);

    int temp1 = response[11]*256;
    int temp2 = response[12];
    output->temp = ((temp1 + temp2) - 435) * 0.1;   //-20.0 .. 65.0 Celsius in 0.1

    output->humidity = response[13]*256 + response[14]; //0..100 %RH
    output->humidity_lvl = get_humidity_lvl(output->humidity);

    uint16_t ch2o_part1 = response[15]*256;
    uint16_t ch2o_part2 = response[16];
    output->ch2o = ch2o_part1 + ch2o_part2; //0..6250 ug/m3
    output->ch2o_lvl = get_ch2o_lvl(output->ch2o);

    uint32_t co_part1 = response[17]*256;
    uint32_t co_part2 = response[18];
    output->co = (co_part1 + co_part2) * 0.1; //0.0 .. 500.0 in 0.1 ppm
    output->co_lvl = get_co_lvl(output->co);

    uint16_t o3_part1 = response[19]*256;
    uint16_t o3_part2 = response[20];
    output->o3 = (o3_part1 + o3_part2) * 10; //0 .. 10000 ppb
    output->o3_lvl = get_o3_lvl(output->o3);

    uint16_t no2_part1 = response[21]*256;
    uint16_t no2_part2 = response[22];
    output->no2 = (no2_part1 + no2_part2) * 10; //0 .. 10 000 ppb in 50 ppb
    output->no2_lvl = get_no2_lvl(output->no2);
}

/*
    return: 0 in case of error, 1 if all is ok.
*/
static int construct_output_message(const struct air_data *d, char *output_message) {
    if (output_message == NULL) {
        return 0;
    }

    int rv = sprintf(output_message, 
            "\n\npm1.0 %s, pm2.5 %s, pm10 %s, CO2 %s, TVOC %s, CH2O %s, CO %s, O3 %s, NO2 %s, RH %s;\n"
            "\npm1.0 %d ug/m3, pm2.5 %d ug/m3, pm10 %d ug/m3, CO2 %d ppm, TVOC %d lvl, CH2O %d ug/m3, CO %.1f ppm, O3 %d ppb, NO2 %d ppb, %.1f *C, %d RH;\n" ,
            lvls[d->pm1_0_lvl], lvls[d->pm2_5_lvl], lvls[d->pm10_lvl], lvls[d->co2_lvl], lvls[d->voc_lvl], lvls[d->ch2o_lvl], lvls[d->co_lvl], lvls[d->o3_lvl], lvls[d->no2_lvl], lvls[d->humidity_lvl],
            d->pm1_0, d->pm2_5, d->pm10, d->co2, d->voc, d->ch2o, d->co, d->o3, d->no2, d->temp, d->humidity);
    if (rv <= 0) {
        output_message[0] = '\0';
        return 0;
    }

    ESP_LOGI("", "%s", output_message); //for debug
    return 1;
}

static void reset_buffers_and_counter(uint8_t *response, int *response_len, char *output_message) {
    if (*response_len < 0) {
        *response_len = BUF_SIZE;
    }
    memset(response, 0, *response_len);
    memset(output_message, 0, RESULT_MESSAGE_SIZE);
    *response_len = 0;
}

/*In accordance to ZPHS01B datasheet 
Check value=(invert( Byte1+Byte2+...+Byte24))+1
Return: 1 if there are errors and 0 if there are no errors.
*/
static uint8_t check_response(uint8_t *response, int response_length) {
    uint8_t checksum = 0;
    const char *TAG = "RESPONSE CHECK";

    if (response_length != RESPONSE_LENGTH) { // RESPONSE_LENGTH is in accordance to datasheet of ZPHS01B, pages 5 and 6 
        ESP_LOGI(TAG, "error in response length: actual %d, expected %d", response_length, RESPONSE_LENGTH);
        return 1;
    }

    for (int i = 1; i < 25; i++) {
        checksum += response[i];
    }
    checksum = ~checksum + 1;

    if (checksum != response[25]) { //25th byte is sent by module a response's checksum
        ESP_LOGI(TAG, "error in response data checksum: actual %02x, calculated %02x", response[25], checksum);
        return 1;
    }
    return 0;
}

void init_and_run_zphs01b(void) {
    xTaskCreate(zphs01b_task, "zphs01_task", TASK_STACK_SIZE, NULL, 10, NULL);
}
