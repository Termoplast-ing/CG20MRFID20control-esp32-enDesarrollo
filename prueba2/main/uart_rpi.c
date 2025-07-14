#include "uart_rpi.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "structyvar.h"

extern SemaphoreHandle_t mutex_config;
extern ConfigTolva config_actual;

#define UART_BUF_SIZE 1024
#define UART_RX 27
#define UART_TX 26

static QueueHandle_t uart_event_queue;

void uart_rpi_event_task(void *arg)
{
    uart_event_t event;
    uint8_t buffer[UART_BUF_SIZE];
    static char json_buffer[256];
    static int pos = 0;

    while (1)
    {
        if (xQueueReceive(uart_event_queue, &event, portMAX_DELAY))
        {
            switch (event.type)
            {
                case UART_DATA:
                    int len = uart_read_bytes(UART_NUM_1, buffer, event.size, pdMS_TO_TICKS(100));
                    printf("UART1: Recibidos %d bytes\n", len);
                    for (int i = 0; i < len; i++)
                    {
                        if (buffer[i] == '\n')
                        {
                            json_buffer[pos] = '\0';

                            if (xSemaphoreTake(mutex_config, pdMS_TO_TICKS(100))) {
                                strncpy(config_actual.ultimo_json, json_buffer, sizeof(config_actual.ultimo_json));
                                // Simular parseo
                                config_actual.id_tolva = 1;
                                config_actual.motor_on = true;
                                xSemaphoreGive(mutex_config);
                                printf("UART1: JSON actualizado: %s\n", json_buffer);
                            } else {
                                printf("UART1: No se pudo tomar el mutex\n");
                            }
                            pos = 0;
                        }
                        else if (pos < sizeof(json_buffer) - 1)
                        {
                            json_buffer[pos++] = buffer[i];
                        }
                    }
                    break;
                case UART_FIFO_OVF:
                case UART_BUFFER_FULL:
                    printf("UART1: Overflow. Limpiando buffers.\n");
                    uart_flush_input(UART_NUM_1);
                    xQueueReset(uart_event_queue);
                    break;
                default:
                    break;
            }
        }
    }
}

void uart_rpi_init(uart_port_t uart_num)
{
    uart_config_t config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(uart_num, &config);
    uart_set_pin(uart_num, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_num, UART_BUF_SIZE * 2, 0, 20, &uart_event_queue, 0);
    xTaskCreate(uart_rpi_event_task, "uart_rpi_event_task", 4096, NULL, 10, NULL);
}