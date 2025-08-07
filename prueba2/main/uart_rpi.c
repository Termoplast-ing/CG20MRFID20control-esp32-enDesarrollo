#include "uart_rpi.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "cJSON.h"
#include "structyvar.h"
#include "reloj.h"
#include "envio_animal_leido.h"

#define UART_BUF_SIZE 1024
#define UART_RX 27
#define UART_TX 26
#define TAG "UART_RPI"

extern SemaphoreHandle_t mutex_config;
extern ConfigTolva config_actual;

extern void procesar_json_animal(const char *json_str);
extern void procesar_json_curva(const char *json_str);
extern void procesar_json_config(const char *json_str);
extern void actualizar_reloj(time_t timestamp);

static QueueHandle_t uart_event_queue;
static char json_buffer[UART_BUF_SIZE * 4];
static int pos = 0;
static bool capturando = false;

void uart_rpi_event_task(void *arg) {
    uart_event_t event;
    uint8_t data[UART_BUF_SIZE];

    while (1) {
        if (xQueueReceive(uart_event_queue, &event, portMAX_DELAY) && event.type == UART_DATA) {
            int len = uart_read_bytes(UART_NUM_1, data, event.size, pdMS_TO_TICKS(100));

            for (int i = 0; i < len; i++) {
                if (!capturando && data[i] == '<' && i + 2 < len && data[i+1] == '<' && data[i+2] == '<') {
                    capturando = true;
                    pos = 0;
                    i += 2;
                    continue;
                }

                if (capturando && data[i] == '>' && i + 2 < len && data[i+1] == '>' && data[i+2] == '>') {
                    json_buffer[pos] = '\0';
                    capturando = false;
                    i += 2;
                    
                    ESP_LOGI(TAG, "JSON recibido: %s", json_buffer);

                    cJSON *root = cJSON_Parse(json_buffer);
                    if (!root) {
                        ESP_LOGE(TAG, "JSON inválido");
                        continue;
                    }

                    if (cJSON_GetObjectItem(root, "calibraciones") || 
                        cJSON_GetObjectItem(root, "caravanas_libres") || 
                        cJSON_GetObjectItem(root, "indice_corporal")) {
                        
                        procesar_json_config(json_buffer);
                        uart_write_bytes(UART_NUM_1, "CONFIG_RECEIVED\n", strlen("CONFIG_RECEIVED\n"));

                    } else if (cJSON_IsArray(root)) {
                        cJSON *first = cJSON_GetArrayItem(root, 0);
                        if (first) {
                            if (cJSON_GetObjectItem(first, "segmentos")) {
                                procesar_json_curva(json_buffer);
                                uart_write_bytes(UART_NUM_1, "CURVA_RECEIVED\n", strlen("CURVA_RECEIVED\n"));
                            } else if (cJSON_GetObjectItem(first, "caravana")) {
                                procesar_json_animal(json_buffer);
                                uart_write_bytes(UART_NUM_1, "ANIMAL_RECEIVED\n", strlen("ANIMAL_RECEIVED\n"));
                            } else {
                                ESP_LOGW(TAG, "JSON no reconocido dentro del array");
                            }
                        }
                    } else if (cJSON_GetObjectItem(root, "timestamp")) {
                        cJSON *ts = cJSON_GetObjectItem(root, "timestamp");
                        if (ts && cJSON_IsNumber(ts)) {
                            time_t t = (time_t)(ts->valuedouble);
                            actualizar_reloj(t);
                            uart_write_bytes(UART_NUM_1, "HORA_RECEBIDA\n", strlen("HORA_RECEBIDA\n"));
                        }
                    } else {
                        ESP_LOGW(TAG, "JSON no reconocido");
                    }

                    cJSON_Delete(root);
                    continue;
                }

                if (capturando && pos < sizeof(json_buffer) - 1) {
                    json_buffer[pos++] = data[i];
                }
            }

        } else if (event.type == UART_FIFO_OVF || event.type == UART_BUFFER_FULL) {
            ESP_LOGW(TAG, "UART overflow o buffer lleno. Limpiando.");
            uart_flush_input(UART_NUM_1);
            xQueueReset(uart_event_queue);
            pos = 0;
            capturando = false;
        }
    }
}

void uart_rpi_init(uart_port_t uart_num) {
    uart_config_t config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(uart_num, &config);
    uart_set_pin(uart_num, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, UART_BUF_SIZE * 2, UART_BUF_SIZE * 2, 20, &uart_event_queue, 0);

    xTaskCreate(uart_rpi_event_task, "uart_rpi_event_task", 8192, NULL, 10, NULL);
}