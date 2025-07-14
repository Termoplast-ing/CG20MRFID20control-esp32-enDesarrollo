// 📁 main.c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "structyvar.h"
#include "modbus_master_raw.h"
#include "uart_rpi.h"
#include "wifi_init.h"

char tarea_modbus = 0;

void task_modbus_comm(void *param)
{
    while (1)
    {
        switch (tarea_modbus)
        {
            case 0: { // Consulta timestamp de animales
                printf("MODBUS: Revisar timestamp de animales\n");

                if (xSemaphoreTake(mutex_animales, pdMS_TO_TICKS(100))) {
                    for (uint8_t i = 0; i < 20; i++) {
                        animales_copia[i] = animales_actual[i];
                    }
                    xSemaphoreGive(mutex_animales);
                } else {
                    printf("MODBUS: No se pudo tomar el mutex_animales\n");
                }

                if (xSemaphoreTake(mutex_Tanimales, pdMS_TO_TICKS(100))) {
                    copia_Tanimales = timestamp_animales;
                    xSemaphoreGive(mutex_Tanimales);
                } else {
                    printf("MODBUS: No se pudo tomar el mutex_Tanimales\n");
                }
                break;
            }

            case 1: {
                // Lógica para otra tarea...
                break;
            }

            // Agregá más casos según necesites...
            default:
                break;
        }

        // Ejemplo: mostrar config_actual
        if (xSemaphoreTake(mutex_config, pdMS_TO_TICKS(100))) {
            printf("MODBUS: Enviando config a Tolva %d (motor: %s)\n",
                   config_actual.id_tolva,
                   config_actual.motor_on ? "ON" : "OFF");

            printf("MODBUS: Último JSON: %s\n", config_actual.ultimo_json);
            xSemaphoreGive(mutex_config);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void app_main(void)
{
    printf("INICIO: Iniciando sistema controlador...\n");

    // Inicialización de mutex
    mutex_animales = xSemaphoreCreateMutex();
    mutex_Tanimales = xSemaphoreCreateMutex();
    mutex_animales_leidos = xSemaphoreCreateMutex();
    mutex_Tanimales_leidos = xSemaphoreCreateMutex();
    mutex_curvas = xSemaphoreCreateMutex();
    mutex_Tcurvas = xSemaphoreCreateMutex();
    mutex_configuracion = xSemaphoreCreateMutex();
    mutex_Tconfiguracion = xSemaphoreCreateMutex();
    mutex_config = xSemaphoreCreateMutex();

    // Inicializar UART y Modbus
    uart_rpi_init(UART_NUM_1);      // UART desde Raspberry Pi
    modbus_master_init(UART_NUM_2); // UART hacia Tolvas
    wifi_init_sta();                // WiFi (si lo usás)

    // Crear tarea
    xTaskCreatePinnedToCore(task_modbus_comm, "modbus_comm", 4096, NULL, 5, NULL, 0);
}
