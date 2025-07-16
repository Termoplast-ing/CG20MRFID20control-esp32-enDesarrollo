// 📁 main.c
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "time.h"
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
                        printf("Animal %d: %s\n", i, animales_actual[i].nombre);
                        printf("fechaServicio: %lld\n", animales_actual[i].fechaServicio);
                        // Copiar datos de animales actuales a copia
                        animales_copia[i] = animales_actual[i];
                        printf("Animal copia %d: %s\n", i, animales_copia[i].nombre);
                        printf("fechaServicio: %lld\n", animales_copia[i].fechaServicio);
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
                for(uint8_t i = 0; i < 16; i++) {
                    printf("Animal %d: %s\n", i, animales_copia[i].nombre);
                }
                printf("envio modbus:");
                printf("fechaServicio: %lld\n", animales_copia[0].fechaServicio);
                tarea51(animales_copia[0], copia_Tanimales); // Enviar datos de animales
                tarea_modbus = 1; // Cambiar a siguiente tarea
                break;
            }

            case 1: {
                printf("MODBUS: Enviar Configuracion\n");
                tarea_modbus = 2; // Cambiar a siguiente tarea
                break;
            }
            case 2: {
                printf("MODBUS: Enviar curvas\n");
                tarea_modbus = 3; // Cambiar a siguiente tarea
                break;
            }
            case 3: {
                printf("MODBUS: Enviar datos RTC\n");
                tarea_modbus = 4; // Cambiar a siguiente tarea
                break;
            }
            case 4: {
                printf("MODBUS: Enviar datos de por si las moscas\n");
                tarea_modbus = 5; // Cambiar a primera tarea
                break;
            }
            // Agregá más casos según necesites...
            default:
            printf("PUTO EL QUE LEE\n");
                tarea_modbus = 0; // Reiniciar a la primera tarea
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
    inicializar_animales_actual_nombre() ;
    printf("INICIO: Iniciando sistema controlador...\n");
printf("fechaServicio: %lld\n", animales_actual[0].fechaServicio);
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
