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
#include "envio_animal_leido.h"
#include "driver/i2c.h"
#include "reloj.h"

char tarea_modbus = 0;

void task_modbus_comm(void *param)
{
    while (1)
    {
        printf("tarea MODBUS: %d\n", tarea_modbus);
        switch (tarea_modbus)
        {
            case 0: { // 
                /*for(uint8_t j=0; j<20; j++){
            printf("DATOS ANIMAL %d\n", j);
            printf("nombre: %s\n", animales_copia[j].nombre);
            printf("tipoCurva: %d\n", animales_copia[j].tipoCurva);
            printf("pesoDosis: %d\n", animales_copia[j].pesoDosis);
            printf("fechaServicio: %lld\n", animales_copia[j].fechaServicio);   
            printf("indiceCorporal: %d\n", animales_copia[j].indiceCorporal);
            printf("agua: %d\n", animales_copia[j].agua);   
            printf("cantDosis: %d\n", animales_copia[j].cantDosis);
            printf("intervaloMin: %d\n", animales_copia[j].intervaloMin);
            printf("========================================\n");

        }*/
                printf("MODBUS: Revisar timestamp de animales\n");

                if (xSemaphoreTake(mutex_animales, pdMS_TO_TICKS(100))) {
                    printf("MODBUS: mutex_animales tomado, copiando animales...\n");
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
                            
                for(uint8_t i = 0; i < 20; i++) {
                    indice=i;
                    tarea51(indice);
                    vTaskDelay(pdMS_TO_TICKS(250));
                }
                                
                tarea_modbus = 1; // Cambiar a siguiente tarea
                break;
            }

            case 1: {// case para mandar configuracion
                if (xSemaphoreTake(mutex_configuracion, pdMS_TO_TICKS(100))) {
                        configuracion_copia = configuracion_actual;
                    xSemaphoreGive(mutex_configuracion);
                } else {
                    printf("MODBUS: No se pudo tomar el mutex_animales\n");
                }
                printf("%d\n", configuracion_copia.calibracionMotor);
                printf("%d\n", configuracion_copia.calibracionAgua);
                printf("%d\n", configuracion_copia.pesoAnimalDesconocido);
                if (xSemaphoreTake(mutex_Tconfiguracion, pdMS_TO_TICKS(100))) {
                    copia_Tconfiguracion = timestamp_configuracion;
                    xSemaphoreGive(mutex_Tconfiguracion);
                } else {
                    printf("MODBUS: No se pudo tomar el mutex_Tanimales\n");
                }
                tarea40();
               // printf("5: %02X 6: %02X\n",response[5], response[6]);
                vTaskDelay(pdMS_TO_TICKS(250));
               // printf("5: %02X 6: %02X\n",response[5], response[6]);
                if(timeOK) {
                    printf("MODBUS: estoy aca\n");
                    timeOK= false; // Reiniciar la variable timeOK
                    vTaskDelay(pdMS_TO_TICKS(250));
                    tarea41();
                    
                }else{
                    printf("MODBUS: completado al enviar datos de animales\n");
                }
                printf("MODBUS: Enviar Configuracion\n");
                tarea_modbus = 2; // Cambiar a siguiente tarea
                break;
            }
            
            case 2: {// case para mandar curvas
                if (xSemaphoreTake(mutex_curvas, pdMS_TO_TICKS(100))) {
                    for (uint8_t i = 0; i < 5; i++) {
                        curvas_copia[i] = curvas_actual[i];
                    }
                    xSemaphoreGive(mutex_curvas);
                } else {
                    printf("MODBUS: No se pudo tomar el mutex_animales\n");
                }

                if (xSemaphoreTake(mutex_Tcurvas, pdMS_TO_TICKS(100))) {
                    copia_Tcurvas = timestamp_curvas;
                    xSemaphoreGive(mutex_Tcurvas);
                } else {
                    printf("MODBUS: No se pudo tomar el mutex_Tanimales\n");
                }
                tarea60();
                vTaskDelay(pdMS_TO_TICKS(250));
                if(timeOK) {
                    timeOK= false; // Reiniciar la variable timeOK
                   // printf("enviando curvas\n");
                    tarea61();
                    vTaskDelay(pdMS_TO_TICKS(250));
                    //printf("fin tarea 61\n");
                    tarea62();
                    vTaskDelay(pdMS_TO_TICKS(250));
                    //printf("fin tarea 62\n");
                    tarea63();
                    vTaskDelay(pdMS_TO_TICKS(250));
                    //printf("fin tarea 63\n");
                    tarea64();
                    vTaskDelay(pdMS_TO_TICKS(250));
                    //printf("fin tarea 64\n");
                    tarea65();
                }else{
                    printf("MODBUS: datos curvas estan actulizados\n");
                }
                printf("MODBUS: Enviar datos curva\n");
                
/*for(uint8_t i=0;i<5;i++){
            printf("imprimiendo curva %d\n", i);
            for(uint8_t j=0;j<17;j++){
              //  printf("Segmento %d: Inicio: %d, Peso Inicio: %d\n", j, curvas_copia[i].segmentos[j].inicio, curvas_copia[i].segmentos[j].pesoInicio);
            }
        }*/
                tarea_modbus = 3; // Cambiar a siguiente tarea
                break;
            
            }
            
            case 3: { // recuperar datos de animales leido
                if (xSemaphoreTake(mutex_animales_leidos, pdMS_TO_TICKS(100))) {
                    for(uint8_t i=0; i<100 ; i++){
                        animales_leidos_copia[i] = animales_leidos_actual[i];
                    }
                    xSemaphoreGive(mutex_animales_leidos);

                    // Imprimir para corroborar que animales_leidos_actual tiene los datos guardados
                    /*printf("Imprimiendo animales_leidos_actual después de copiar:\n");
                    for (uint8_t i = 0; i < 100; i++) {
                        if (strcmp(animales_leidos_actual[i].nombre, "000000000000000") != 0) {
                            printf("Animal %d: %s\n", i, animales_leidos_actual[i].nombre);
                            printf("  Fecha dispensado: %lld\n", animales_leidos_actual[i].fechaDispensado);
                            printf("  Peso dispensado: %d\n", animales_leidos_actual[i].pesoDispensado);
                            printf("  Nro Tolva: %d\n", animales_leidos_actual[i].nroTolva);
                        }
                    }*/ 
                } else {
                    printf("MODBUS: No se pudo tomar el mutex_Tanimales\n");
                }               
                tarea20();
                vTaskDelay(pdMS_TO_TICKS(250));
                /* if(response[5]==0xff && response[6]==0xff) {
                    for(uint8_t i = 0; i < 9; i++) {
                        response[i]=0;
                    }
                    tarea81();
                }*/
                for (uint8_t i = 0; i < 100; i++) {
                    if (strcmp(animales_leidos_copia[i].nombre, "000000000000000") != 0) {
                        printf("Animal %d: %s\n", i, animales_leidos_copia[i].nombre);
                        printf("Fecha dispensado: %lld\n", animales_leidos_copia[i].fechaDispensado);
                        printf("Peso dispensado: %02x\n", animales_leidos_copia[i].pesoDispensado);
                        printf("Nro Tolva: %02x\n", animales_leidos_copia[i].nroTolva);
                    }
                }
                tarea_modbus = 4; // Cambiar a primera tarea
                break;
            }
        
            case 4: { // envion RTC
                read_time();
 printf("Hora actual: %02d:%02d:%02d\n", RTC_hora.tm_hour, RTC_hora.tm_min, RTC_hora.tm_sec);
    printf("Fecha actual: %02d/%02d/%04d\n", RTC_hora.tm_mday, RTC_hora.tm_mon + 1, RTC_hora.tm_year + 1900);
                if (RTC_hora.tm_hour == 0) {
                    if(envio_RTC){
                        tarea70();
                        envio_RTC = false; // Reiniciar la variable envio_RTC
                    }
                }else{
                    envio_RTC = true; // Indicar que se debe enviar el RTC
                }        
                tarea_modbus = 5; // Cambiar a primera tarea
                break;
            }
            // Agregá más casos según necesites...

            case 5: { // Enviar animales leídos por UART
                enviar_animales_leidos_uart();
                tarea_modbus = 0; // Reiniciar a la primera tarea
                break;
            }

            default:{
                tarea_modbus = 0; // Reiniciar a la primera tarea
                break;
            }
        }
        
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    inicializar_animales_actual_nombre() ;
    inicializar_config();
    inicializar_curvas();
    for(uint8_t i=0; i<100; i++){
    snprintf(animales_leidos_actual[i].nombre, sizeof(animales_leidos_actual[i].nombre), "000000000000000");
    }
    

    //printf("curvas_actual[2].segmentos[5].inicio: %d\n", curvas_actual[2].segmentos[5].inicio);
    printf("INICIO: Iniciando sistema controlador...\n");
    ///printf("fechaServicio: %lld\n", animales_actual[0].fechaServicio);
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
    wifi_init_sta();    
    init_i2c();            // WiFi (si lo usás)

    // Crear tarea
    xTaskCreatePinnedToCore(task_modbus_comm, "modbus_comm", 4096, NULL, 5, NULL, 0);
}