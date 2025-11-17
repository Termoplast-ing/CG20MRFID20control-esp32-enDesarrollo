//  main.c
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

char tarea_modbus = 100;

void task_modbus_comm(void *param)
{
    
    while (1)
    {
        printf("tarea MODBUS: %d\n", tarea_modbus);
        switch (tarea_modbus)
        {
            case 1: { // 
                
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
                for(uint8_t j=0; j<20; j++){
                    printf("DATOS ANIMAL %d\n", j);
                    printf("nombre: %s copia: %s\n", animales_copia[j].nombre, animales_actual[j].nombre);
                    printf("tipoCurva: %d\n", animales_copia[j].tipoCurva);
                    printf("pesoDosis: %d\n", animales_copia[j].pesoDosis);
                    printf("fechaServicio: %lld\n", animales_copia[j].fechaServicio);   
                    printf("indiceCorporal: %d\n", animales_copia[j].indiceCorporal);
                    printf("agua: %d\n", animales_copia[j].agua);   
                    printf("cantDosis: %d\n", animales_copia[j].cantDosis);
                    printf("intervaloMin: %d\n", animales_copia[j].intervaloMin);
                    printf("========================================\n");
        
                }       
                for(uint8_t i = 0; i < 20; i++) {
                    printf("Enviando animal %lld...\n", copia_Tanimales);
                    indice=i;
                    tarea51(indice);
                    vTaskDelay(pdMS_TO_TICKS(500));
                }
                                
                tarea_modbus = 2; // Cambiar a siguiente tarea
                break;
            }

            case 2: {// case para mandar configuracion
                if (xSemaphoreTake(mutex_configuracion, pdMS_TO_TICKS(100))) {
                        configuracion_copia = configuracion_actual;
                    xSemaphoreGive(mutex_configuracion);
                } else {
                    printf("MODBUS: No se pudo tomar el mutex_animales\n");
                }
                printf("%d\n", configuracion_copia.calibracionMotor);
                printf("%d\n", configuracion_copia.calibracionAgua);
                printf("%d\n", configuracion_copia.pesoAnimalDesconocido);
                printf("Caravana Libre 1 :");
                for(uint8_t i = 0; i < 16; i++) {
                    printf("%c", configuracion_copia.caravanaLibre1[i]);
                }
                printf("\n");
                printf("Caravana Libre 2 :");
                for(uint8_t i = 0; i < 16; i++) {
                    printf("%c", configuracion_copia.caravanaLibre2[i]);
                }
                printf("\n");
                printf("Caravana Libre 3 :");
                for(uint8_t i = 0; i < 16; i++) {
                    printf("%c", configuracion_copia.caravanaLibre3[i]);
                }
                printf("\n");
                printf("Caravana Libre 4 :");
                for(uint8_t i = 0; i < 16; i++) {
                    printf("%c", configuracion_copia.caravanaLibre4[i]);
                }
                printf("\n");
                printf("Caravana Libre 5 :");
                for(uint8_t i = 0; i < 16; i++) {
                    printf("%c", configuracion_copia.caravanaLibre5[i]);
                }
                printf("\n");
                
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
               printf("timeOK: %d\n", timeOK);
                if(timeOK) {
                    printf("MODBUS: estoy aca\n");
                    timeOK= 0; // Reiniciar la variable timeOK
                    vTaskDelay(pdMS_TO_TICKS(250));
                    tarea41();
                    
                }else{
                    printf("MODBUS: completado al enviar datos de animales\n");
                }
                printf("MODBUS: Enviar Configuracion\n");
                tarea_modbus = 3; // Cambiar a siguiente tarea
                break;
            }
            
           /* case 2: {// case para mandar curvas
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
                
    for(uint8_t i=0;i<5;i++){
            printf("imprimiendo curva %d\n", i);
            for(uint8_t j=0;j<17;j++){
              //  printf("Segmento %d: Inicio: %d, Peso Inicio: %d\n", j, curvas_copia[i].segmentos[j].inicio, curvas_copia[i].segmentos[j].pesoInicio);
            }
        }*//*
                tarea_modbus = 3; // Cambiar a siguiente tarea
                break;
            
            }*/
            
            case 3: { // recuperar datos de animales leido
                                tarea20();
                vTaskDelay(pdMS_TO_TICKS(250));
                               char msg[85];
              /*  if (xSemaphoreTake(mutex_animales_leidos, pdMS_TO_TICKS(100))) {
                    for(uint8_t i=0; i<100 ; i++){
                        animales_leidos_copia[i] = animales_leidos_actual[i];
                    }
                    xSemaphoreGive(mutex_animales_leidos);
    */
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
               // } else {
                //    printf("MODBUS: No se pudo tomar el mutex_Tanimales\n");
                //}               

                /* if(response[5]==0xff && response[6]==0xff) {
                    for(uint8_t i = 0; i < 9; i++) {
                        response[i]=0;
                    }
                    tarea81();
                }*/

                for (uint8_t i = 0; i < 100; i++) {
                    if ((strcmp(animales_leidos_actual[i].nombre, "000000000000000") == 0)|| (animales_leidos_actual[i].nombre[0] == '\0')) {
                        if(i>0){
                            snprintf(msg, sizeof(msg),
                                "<<<[{\"caravana\":\"%s\",\"fecha\":%lld,\"peso\":%d,\"corral\":%d}]>>>",
                                animales_leidos_actual[i-1].nombre, animales_leidos_actual[i-1].fechaDispensado, animales_leidos_actual[i-1].pesoDispensado, animales_leidos_actual[i-1].nroTolva);
                            printf("Envio La CARAVANA: %s\n", msg);
                            uart_flush_input(UART_NUM_1);
                            vTaskDelay(pdMS_TO_TICKS(100));
                            uart_write_bytes(UART_NUM_1, msg, strlen(msg));
                            vTaskDelay(pdMS_TO_TICKS(500));
                            //animales_leidos_actual[i-1]= {0};
                            strncpy(animales_leidos_actual[i-1].nombre, "000000000000000", sizeof(animales_leidos_actual[i-1].nombre));
                            //animales_leidos_actual[i-1].nombre[15] = '\0';
                            animales_leidos_actual[i-1].fechaDispensado = 0;
                            animales_leidos_actual[i-1].pesoDispensado = 0;
                            animales_leidos_actual[i-1].nroTolva = 0;
                            break;       
                        }
                        break; 
                    }                    
                }
                tarea_modbus = 4; // Cambiar a primera tarea
                break;
            }
        
            case 0: { // envion RTC
                read_time();
                printf("timestamp actual esp: %lld\n", time(NULL));
                printf("timestamp actual rtc: %lld\n", RTC_time);
 printf("Hora actual: %02d:%02d:%02d\n", RTC_hora.tm_hour, RTC_hora.tm_min, RTC_hora.tm_sec);
    printf("Fecha actual: %02d/%02d/%04d\n", RTC_hora.tm_mday, RTC_hora.tm_mon + 1, RTC_hora.tm_year + 1900);
    /*if (RTC_hora.tm_hour == 0) {
                    if(envio_RTC){
                        printf("holaRTC\n");
                        tarea70();
                        envio_RTC = false; // Reiniciar la variable envio_RTC
                    }else{
                        if(actualizarRTC){
                            tarea70();
                            actualizarRTC = false; // Reiniciar la variable actualizarRTC
                        }
                    }
                }else{
                    if(actualizarRTC){
                        tarea70();
                        actualizarRTC = false; // Reiniciar la variable actualizarRTC
                    }
                    envio_RTC = true; // Indicar que se debe enviar el RTC
                }*/

tarea70();
                tarea_modbus = 1; // Cambiar a primera tarea
                break;
            }
            // Agregá más casos según necesites...

            case 4: { // Enviar animales leídos por UART
                enviar_animales_leidos_uart();
                tarea_modbus = 0; // Reiniciar a la primera tarea
                break;
            }

            // Agregá más casos según necesites... NUEVO CASE
            case 100: {  // Enviar hora RTC a Raspberry en formato legible
                // Leer hora del RTC externo
                read_time();

                // Verificar si los segundos son inválidos (ej: 80 por pila agotada)
               
                if (RTC_hora.tm_sec > 59) {
                    printf("RTC inválido detectado (segundos=%d). Reiniciando a fecha base...\n", RTC_hora.tm_sec);

                    // Configurar fecha base: 2000-01-01 00:00:00
                    RTC_hora.tm_year = 2000 - 1900; // struct tm cuenta desde 1900
                    RTC_hora.tm_mon  = 0;           // enero = 0
                    RTC_hora.tm_mday = 1;
                    RTC_hora.tm_hour = 0;
                    RTC_hora.tm_min  = 0;
                    RTC_hora.tm_sec  = 0;

                    // Convertir a time_t y escribir al RTC
                    //time_t base_time = mktime(&RTC_hora);
                    actualizar_reloj(RTC_time);

                    // Leer de nuevo para confirmar
                    read_time();
                }

                /* if rtc en segundos 80
                    voy a escribir reloj con 1/1/2000 00:00:00
                    y despues vuelvo a leer*/

                // Preparar buffer para UART
                char buffer[64];
                snprintf(buffer, sizeof(buffer),
                        "<%04d-%02d-%02d %02d:%02d:%02d>",
                        RTC_hora.tm_year + 1900,
                        RTC_hora.tm_mon + 1,
                        RTC_hora.tm_mday,
                        RTC_hora.tm_hour,
                        RTC_hora.tm_min,
                        RTC_hora.tm_sec);

                // Limpiar buffer de entrada UART
                printf("Enviando hora RTC a Raspberry: %s\n", buffer);
                uart_flush_input(UART_NUM_1);

                // Bucle: enviar timestamp hasta recibir "OK"
                //char respuesta[3] = {0};
                while (1) {
                    uart_write_bytes(UART_NUM_1, buffer, strlen(buffer));

                    //int len = uart_read_bytes(UART_NUM_1, (uint8_t*)respuesta, 2, pdMS_TO_TICKS(500));
                    
                    //int len=0;
                    //if (len == 2 && strncmp(respuesta, "OK", 2) == 0) {
                    printf("%s\n", respuesta);
                    if (strncmp(respuesta, "OK", 2) == 0) {
                    printf("OK recibido, continuando...\n");
                        break;
                    }
                    
                    /*if (len == 2 && strncmp(respuesta, "OK", 2) == 0) {
                        printf("OK recibido, continuando...\n");
                        break;
                    }*/

                    vTaskDelay(pdMS_TO_TICKS(500)); // esperar antes de reintentar
                }

                tarea_modbus = 0;  // continuar con la siguiente tarea
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
    //inicializar_animales_actual_nombre() ;
    //inicializar_config();
    //inicializar_curvas();
    for(uint8_t i=0; i<100; i++){
    snprintf(animales_leidos_actual[i].nombre, sizeof(animales_leidos_actual[i].nombre), "000000000000000");
    }
    
vTaskDelay(pdMS_TO_TICKS(2000));
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