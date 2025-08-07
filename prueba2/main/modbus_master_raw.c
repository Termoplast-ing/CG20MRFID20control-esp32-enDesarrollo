#include "modbus_master_raw.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "freertos/queue.h"
#include "structyvar.h"
#include "reloj.h"


// Define CONFIG_FREERTOS_HZ if not defined (default 100 Hz)
#ifndef CONFIG_FREERTOS_HZ
#define CONFIG_FREERTOS_HZ 100
#endif
#define TIMEOUT_MS 1000
#define UART_NUM2 UART_NUM_2
#define BUF_SIZE 1024
#define TXD_PIN 17
#define RXD_PIN 16
#define SLAVE_ADDR 0x02
#define MAX_ANIMALES 100

//static const char *TAG = "MODBUS_MASTER";

uint16_t modbus_crc16(const uint8_t *buf, int len);

void send_modbus_request(const uint8_t *frame, int len) {

    gpio_set_level(GPIO_NUM_4, 1);
    vTaskDelay(pdMS_TO_TICKS(2));
    uart_write_bytes(UART_NUM2, (const char *)frame, len);
    uart_wait_tx_done(UART_NUM2, pdMS_TO_TICKS(100));
    gpio_set_level(GPIO_NUM_4, 0);
}

int receive_modbus_response(uint8_t *buf, int maxlen) {

    int len = uart_read_bytes(UART_NUM2, buf, maxlen, pdMS_TO_TICKS(200));
    if (len > 0) {
        for (int i = 0; i < len; i++) printf("%02X ", buf[i]);
    }
    return len;
}

void modbus_master_task(void *arg) {


    
   /* uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    // --- Ejemplo: Escribir un registro (0x06) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x06;
    frame[2] = 0x00;
    frame[3] = 0x01; // Dirección del registro
    frame[4] = 0x00;
    frame[5] = 0x04; // Valor a escribir
    
    crc = modbus_crc16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;
    send_modbus_request(frame, 8);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);


    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    printf("ahora si que si\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    printf("delay\n");*/
    vTaskDelete(NULL);
}

void modbus_master_init(uart_port_t uart_num)
{
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, 0);

    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM2, &uart_config);
    uart_set_pin(UART_NUM2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM2, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0);

    xTaskCreate(modbus_master_task, "modbus_master_task", 4096, NULL, 10, NULL);
}

// --- Implementación estándar de CRC16 Modbus ---
uint16_t modbus_crc16(const uint8_t *buf, int len) {
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
//void imprimir_animales_copia(void);
void tarea51(uint8_t index) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

      // Print para verificar datos que vamos a enviar
   /* printf("DEBUG - Enviando datos animal índice %d:\n", index);
    printf("  nombre: %s\n", animales_copia[index].nombre);
    printf("  tipoCurva: %d\n", animales_copia[index].tipoCurva);
    printf("  pesoDosis: %d\n", animales_copia[index].pesoDosis);
    printf("  fechaServicio: %lld\n", animales_copia[index].fechaServicio);
    printf("  indiceCorporal: %d\n", animales_copia[index].indiceCorporal);
    printf("  agua: %d\n", animales_copia[index].agua);
    printf("  cantDosis: %d\n", animales_copia[index].cantDosis);
    printf("  intervaloMin: %d\n", animales_copia[index].intervaloMin);*/

    // --- Ejemplo: Escribir un registro (0x06) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x51; // Función: escribir datos de dieta de caravanas
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = index;
    frame[5] = 25; // Cantidad de registros
    frame[6] = 49; // Cantidad de bytes (2 registros x 2 bytes)
    
    // String de número caravana
    //snprintf((char *)&frame[17], sizeof(frame) - 17, "%s", animal.nombre);
   // printf("enviado animal: %d\n", index);
        // Timestamp de central
        
            frame[7] = (copia_Tanimales >> 56) & 0xFF;
            frame[8] = (copia_Tanimales >> 48) & 0xFF;
            frame[9] = (copia_Tanimales >> 40) & 0xFF;
            frame[10] = (copia_Tanimales >> 32) & 0xFF;
            frame[11] = (copia_Tanimales >> 24) & 0xFF;
            frame[12] = (copia_Tanimales >> 16) & 0xFF;
            frame[13] = (copia_Tanimales >> 8) & 0xFF;
            frame[14] = copia_Tanimales & 0xFF;
     
    frame[15] = animales_copia[index].nombre[0]; frame[16] = animales_copia[index].nombre[1];   // string de numero caravana
    frame[17] = animales_copia[index].nombre[2]; frame[18] = animales_copia[index].nombre[3];   // pasado por caracter 
    frame[19] = animales_copia[index].nombre[4]; frame[20] = animales_copia[index].nombre[5];   // de a un byte
    frame[21] = animales_copia[index].nombre[6]; frame[22] = animales_copia[index].nombre[7];   // 
    frame[23] = animales_copia[index].nombre[8]; frame[24] = animales_copia[index].nombre[9];   // tamaño 16 bytes
    frame[25] = animales_copia[index].nombre[10]; frame[26] = animales_copia[index].nombre[11];   // 
    frame[27] = animales_copia[index].nombre[12]; frame[28] = animales_copia[index].nombre[13];   // desde dir. 15
    frame[29] = animales_copia[index].nombre[14]; frame[30] = '\0';  // hasta dir. 30

    frame[31] = animales_copia[index].tipoCurva; frame[32] = animales_copia[index].pesoDosis;  // dir. 31 = tipo de curva // dir. 32 = peso dosis
    frame[33] = (animales_copia[index].fechaServicio >> 56) & 0xFF; frame[34] = (animales_copia[index].fechaServicio >> 48) & 0xFF;  // Timestamp de fecha Inseminacion
    frame[35] = (animales_copia[index].fechaServicio >> 40) & 0xFF; frame[36] = (animales_copia[index].fechaServicio >> 32) & 0xFF;  // tamaño 8 bytes
    frame[37] = (animales_copia[index].fechaServicio >> 24) & 0xFF; frame[38] = (animales_copia[index].fechaServicio>> 16) & 0xFF;  // desde dir. 33
    frame[39] = (animales_copia[index].fechaServicio >> 8) & 0xFF; frame[40] = animales_copia[index].fechaServicio & 0xFF;  // hasta dir. 40

    frame[41] = animales_copia[index].indiceCorporal; frame[42] = animales_copia[index].agua;  // dir. 41 = indice corporal // dir. 42 = booleano del agua
    frame[43] = animales_copia[index].cantDosis; frame[44] = ((animales_copia[index].intervaloMin >> 8) & 0xFF);  // dir. 43 = cantidad dosis // dir. 44 = intervalomin.(byte alto)
    frame[45] = (animales_copia[index].intervaloMin & 0xFF); frame[46] = 0x00;    // dir. 44 = intervalomin.(byte bajo)
    
   /* printf("numero caravana: %s\n", animales_copia[index].nombre);
    printf("tipoCurva: %d\n", animales_copia[index].tipoCurva);
    printf("pesoDosis: %d\n", animales_copia[index].pesoDosis);
    printf("fechaServicio: %lld\n", animales_copia[index].fechaServicio);
    printf("indiceCorporal: %d\n", animales_copia[index].indiceCorporal);
    printf("agua: %d\n", animales_copia[index].agua);
    printf("cantDosis: %d\n", animales_copia[index].cantDosis);
    printf("intervaloMin: %d\n", animales_copia[index].intervaloMin);
    */
    crc = modbus_crc16(frame, 47);
    frame[47] = crc & 0xFF; // CRC byte bajo
    frame[48] = crc >> 8;   // CRC byte alto
    /*for (int i = 0; i < 49; i++) {
        printf("%02X ", frame[i]);
    }*/
   // printf("\n");
    send_modbus_request(frame, 49);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    //printf("DEBUG - Respuesta recibida: ");
   /*for (int i = 0; i < BUF_SIZE; i++) {
        printf("%02X ", response[i]);
    }*/
   // printf("\n");
}

void tarea40() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x40; // Función: escribir datos de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 10; // Cantidad de registros
    frame[6] = 20; // Cantidad de bytes (2 registros x 2 bytes)

         // Timestamp de central
    frame[7] = (copia_Tconfiguracion >> 56) & 0xFF;
    frame[8] = (copia_Tconfiguracion >> 48) & 0xFF;
    frame[9] = (copia_Tconfiguracion >> 40) & 0xFF;
    frame[10] = (copia_Tconfiguracion >> 32) & 0xFF;
    frame[11] = (copia_Tconfiguracion >> 24) & 0xFF;
    frame[12] = (copia_Tconfiguracion >> 16) & 0xFF;
    frame[13] = (copia_Tconfiguracion >> 8) & 0xFF;
    frame[14] = copia_Tconfiguracion & 0xFF;
    frame[15] = configuracion_copia.calibracionMotor;
    frame[16] = configuracion_copia.calibracionAgua;
    frame[17] = configuracion_copia.pesoAnimalDesconocido;

     crc = modbus_crc16(frame, 18);
    frame[18] = crc & 0xFF; // CRC byte bajo
    frame[19] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 20);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    if(response[5] == 0xff && response[6] == 0xff) {
        timeOK= true; // Indicar que el tiempo está sincronizado
    } else {
        timeOK = false; // Indicar que el tiempo no está sincronizado
    }
}

void tarea41() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x41; // Función: escribir datos de caravanas libres de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 45; // Cantidad de registros
    frame[6] = 89; // Cantidad de bytes (2 registros x 2 bytes)



    frame[7] = configuracion_copia.caravanaLibre1[0]; frame[8] = configuracion_copia.caravanaLibre1[1];
    frame[9] = configuracion_copia.caravanaLibre1[2]; frame[10] = configuracion_copia.caravanaLibre1[3];
    frame[11] = configuracion_copia.caravanaLibre1[4]; frame[12] = configuracion_copia.caravanaLibre1[5];
    frame[13] = configuracion_copia.caravanaLibre1[6]; frame[14] = configuracion_copia.caravanaLibre1[7];
    frame[15] = configuracion_copia.caravanaLibre1[8]; frame[16] = configuracion_copia.caravanaLibre1[9];
    frame[17] = configuracion_copia.caravanaLibre1[10]; frame[18] = configuracion_copia.caravanaLibre1[11];
    frame[19] = configuracion_copia.caravanaLibre1[12]; frame[20] = configuracion_copia.caravanaLibre1[13];
    frame[21] = configuracion_copia.caravanaLibre1[14]; frame[22] = configuracion_copia.caravanaLibre1[15]; 
    frame[23] = configuracion_copia.caravanaLibre2[0]; frame[24] = configuracion_copia.caravanaLibre2[1];
    frame[25] = configuracion_copia.caravanaLibre2[2]; frame[26] = configuracion_copia.caravanaLibre2[3];
    frame[27] = configuracion_copia.caravanaLibre2[4]; frame[28] = configuracion_copia.caravanaLibre2[5];
    frame[29] = configuracion_copia.caravanaLibre2[6]; frame[30] = configuracion_copia.caravanaLibre2[7];
    frame[31] = configuracion_copia.caravanaLibre2[8]; frame[32] = configuracion_copia.caravanaLibre2[9];
    frame[33] = configuracion_copia.caravanaLibre2[10]; frame[34] = configuracion_copia.caravanaLibre2[11];
    frame[35] = configuracion_copia.caravanaLibre2[12]; frame[36] = configuracion_copia.caravanaLibre2[13];
    frame[37] = configuracion_copia.caravanaLibre2[14]; frame[38] = configuracion_copia.caravanaLibre2[15]; 
    frame[39] = configuracion_copia.caravanaLibre3[0]; frame[40] = configuracion_copia.caravanaLibre3[1];
    frame[41] = configuracion_copia.caravanaLibre3[2]; frame[42] = configuracion_copia.caravanaLibre3[3];
    frame[43] = configuracion_copia.caravanaLibre3[4]; frame[44] = configuracion_copia.caravanaLibre3[5];
    frame[45] = configuracion_copia.caravanaLibre3[6]; frame[46] = configuracion_copia.caravanaLibre3[7];
    frame[47] = configuracion_copia.caravanaLibre3[8]; frame[48] = configuracion_copia.caravanaLibre3[9];
    frame[49] = configuracion_copia.caravanaLibre3[10]; frame[50] = configuracion_copia.caravanaLibre3[11];
    frame[51] = configuracion_copia.caravanaLibre3[12]; frame[52] = configuracion_copia.caravanaLibre3[13];
    frame[53] = configuracion_copia.caravanaLibre3[14]; frame[54] = configuracion_copia.caravanaLibre3[15]; 
    frame[55] = configuracion_copia.caravanaLibre4[0]; frame[56] = configuracion_copia.caravanaLibre4[1];
    frame[57] = configuracion_copia.caravanaLibre4[2]; frame[58] = configuracion_copia.caravanaLibre4[3];
    frame[59] = configuracion_copia.caravanaLibre4[4]; frame[60] = configuracion_copia.caravanaLibre4[5];
    frame[61] = configuracion_copia.caravanaLibre4[6]; frame[62] = configuracion_copia.caravanaLibre4[7];
    frame[63] = configuracion_copia.caravanaLibre4[8]; frame[64] = configuracion_copia.caravanaLibre4[9];
    frame[65] = configuracion_copia.caravanaLibre4[10]; frame[66] = configuracion_copia.caravanaLibre4[11];
    frame[67] = configuracion_copia.caravanaLibre4[12]; frame[68] = configuracion_copia.caravanaLibre4[13];
    frame[69] = configuracion_copia.caravanaLibre4[14]; frame[70] = configuracion_copia.caravanaLibre4[15]; 
    frame[71] = configuracion_copia.caravanaLibre5[0]; frame[72] = configuracion_copia.caravanaLibre5[1];
    frame[73] = configuracion_copia.caravanaLibre5[2]; frame[74] = configuracion_copia.caravanaLibre5[3];
    frame[75] = configuracion_copia.caravanaLibre5[4]; frame[76] = configuracion_copia.caravanaLibre5[5];
    frame[77] = configuracion_copia.caravanaLibre5[6]; frame[78] = configuracion_copia.caravanaLibre5[7];
    frame[79] = configuracion_copia.caravanaLibre5[8]; frame[80] = configuracion_copia.caravanaLibre5[9];
    frame[81] = configuracion_copia.caravanaLibre5[10]; frame[82] = configuracion_copia.caravanaLibre5[11];
    frame[83] = configuracion_copia.caravanaLibre5[12]; frame[84] = configuracion_copia.caravanaLibre5[13];
    frame[85] = configuracion_copia.caravanaLibre5[14]; frame[86] = configuracion_copia.caravanaLibre5[15]; 
    
    crc = modbus_crc16(frame, 87);
    frame[87] = crc & 0xFF; // CRC byte bajo
    frame[88] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 89);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    
}

void tarea60() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x60; // Función: escribir datos de caravanas libres de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

              // Timestamp de central
    frame[7] = (copia_Tcurvas >> 56) & 0xFF;
    frame[8] = (copia_Tcurvas >> 48) & 0xFF;
    frame[9] = (copia_Tcurvas >> 40) & 0xFF;
    frame[10] = (copia_Tcurvas >> 32) & 0xFF;
    frame[11] = (copia_Tcurvas >> 24) & 0xFF;
    frame[12] = (copia_Tcurvas >> 16) & 0xFF;
    frame[13] = (copia_Tcurvas >> 8) & 0xFF;
    frame[14] = copia_Tcurvas & 0xFF;

    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    if(response[5] == 0xff && response[6] == 0xff) {
        timeOK= true; // Indicar que el tiempo está sincronizado
    } else {
        timeOK = false; // Indicar que el tiempo no está sincronizado
    }
}

void tarea61() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;
    uint8_t pos = 7; // Posición inicial para los datos de caravanas libres

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x61; // Función: escribir datos de caravanas libres de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 22; // Cantidad de registros
    frame[6] = 43; // Cantidad de bytes (2 registros x 2 bytes)

    pos = 7;    
    for(int i=0; i<17; i++) {
        frame[pos++] = curvas_copia[0].segmentos[i].inicio;
        frame[pos++] = curvas_copia[0].segmentos[i].pesoInicio; 
    }

    crc = modbus_crc16(frame, 41);
    frame[41] = crc & 0xFF; // CRC byte bajo
    frame[42] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 43);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
}

void tarea62() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x62; // Función: escribir datos de caravanas libres de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 22; // Cantidad de registros
    frame[6] = 43; // Cantidad de bytes (2 registros x 2 bytes)

    uint8_t pos = 7;    
    for(int i=0; i<17; i++) {
        frame[pos++] = curvas_copia[1].segmentos[i].inicio;
        frame[pos++] = curvas_copia[1].segmentos[i].pesoInicio; 
    }

    crc = modbus_crc16(frame, 41);
    frame[41] = crc & 0xFF; // CRC byte bajo
    frame[42] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 43);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
}

void tarea63() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;
    uint8_t pos = 7; // Posición inicial para los datos de caravanas libres

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x63; // Función: escribir datos de caravanas libres de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 22; // Cantidad de registros
    frame[6] = 43; // Cantidad de bytes (2 registros x 2 bytes)

    pos = 7;    
    for(int i=0; i<17; i++) {
        frame[pos++] = curvas_copia[2].segmentos[i].inicio;
        frame[pos++] = curvas_copia[2].segmentos[i].pesoInicio; 
    }

    crc = modbus_crc16(frame, 41);
    frame[41] = crc & 0xFF; // CRC byte bajo
    frame[42] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 43);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
}

void tarea64() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;
    uint8_t pos = 7; // Posición inicial para los datos de caravanas libres

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x64; // Función: escribir datos de caravanas libres de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 22; // Cantidad de registros
    frame[6] = 43; // Cantidad de bytes (2 registros x 2 bytes)

    pos = 7;    
    for(int i=0; i<17; i++) {
        frame[pos++] = curvas_copia[3].segmentos[i].inicio;
        frame[pos++] = curvas_copia[3].segmentos[i].pesoInicio; 
    }

    crc = modbus_crc16(frame, 41);
    frame[41] = crc & 0xFF; // CRC byte bajo
    frame[42] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 43);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
}

void tarea65() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;
        uint8_t pos = 7; // Posición inicial para los datos de caravanas libres

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x65; // Función: escribir datos de caravanas libres de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 22; // Cantidad de registros
    frame[6] = 43; // Cantidad de bytes (2 registros x 2 bytes)

    pos = 7;    
    for(int i=0; i<17; i++) {
        frame[pos++] = curvas_copia[4].segmentos[i].inicio;
        frame[pos++] = curvas_copia[4].segmentos[i].pesoInicio; 
    }

    crc = modbus_crc16(frame, 41);
    frame[41] = crc & 0xFF; // CRC byte bajo
    frame[42] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 43);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
}

void tarea70() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;
    time_t tiempo = time(NULL);

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x70; // Función: escribir datos de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

             // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);

    RTC_time = tiempo; // Guardar el tiempo actual en la variable global
    printf("Tiempo RTC actualizado: %lld\n", RTC_time);

    localtime_r(&RTC_time, &RTC_hora); // Convertir el tiempo a la estructura localtime

    ds1307_write_register(0x00, RTC_hora.tm_sec); // segundos
    ds1307_write_register(0x01, RTC_hora.tm_min); // minutos
    ds1307_write_register(0x02, RTC_hora.tm_hour); // horas
    ds1307_write_register(0x04, RTC_hora.tm_wday + 1); // día de la semana (1-7)
    ds1307_write_register(0x05, RTC_hora.tm_mon + 1); // mes (1-12)
    ds1307_write_register(0x06, RTC_hora.tm_year - 100); // año (a partir de 2000)
    ds1307_write_register(0x07,0x93);
    
        
}

void tarea20() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x20; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 4; // Cantidad de registros
    frame[6] = 8; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 0x00;

    /*// Timestamp de central
    frame[7] = (copia_Tanimales_leidos >> 56) & 0xFF;
    frame[8] = (copia_Tanimales_leidos >> 48) & 0xFF;
    frame[9] = (copia_Tanimales_leidos >> 40) & 0xFF;
    frame[10] = (copia_Tanimales_leidos >> 32) & 0xFF;
    frame[11] = (copia_Tanimales_leidos >> 24) & 0xFF;
    frame[12] = (copia_Tanimales_leidos >> 16) & 0xFF;
    frame[13] = (copia_Tanimales_leidos >> 8) & 0xFF;
    frame[14] = copia_Tanimales_leidos & 0xFF;*/
    
    crc = modbus_crc16(frame, 8);
    frame[8] = crc & 0xFF; // CRC byte bajo
    frame[9] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 10);
    vTaskDelay(pdMS_TO_TICKS(250));
    receive_modbus_response(response, BUF_SIZE);  


    for (uint8_t i = 0; i < MAX_ANIMALES; i++) {
        
        if (strcmp(animales_leidos_copia[i].nombre, "000000000000000") == 0) {
            // Encontrado espacio libre, copiar auxiliar
            for(uint8_t j = 0; j < 16; j++) {
                printf("nombre: %c", response[j+7]);
                //printf("\n");
                animales_leidos_copia[i].nombre[j] = response[j+7];
            }printf("\n");
            

            for(int k = 0; k < 8; k++) {
                animales_leidos_copia[i].fechaDispensado = animales_leidos_copia[i].fechaDispensado | ((time_t)response[23+k] << (8 * (7 - k)));
            }
            animales_leidos_copia[i].pesoDispensado = response[31];
            animales_leidos_copia[i].nroTolva= response[0];

            printf("1Animal agregado en la posición %d: %s\n", i, animales_leidos_copia[i].nombre);
            printf("1Fecha dispensado: %lld\n", animales_leidos_copia[i].fechaDispensado);
            printf("1Peso dispensado: %d\n", animales_leidos_copia[i].pesoDispensado);
            printf("1Nro Tolva: %d\n", animales_leidos_copia[i].nroTolva);
            break;
        }
    }

}


/*void tarea81() {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;
    data_animal_leido animales_leidos_aux;
    int total_animales_leidos = 0;
    // --- Ejemplo: Leer 4 registros desde el esclavo ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x03; // Función: Leer registros
    frame[2] = 0x00; // Dirección alta
    frame[3] = 0x00; // Dirección baja
    frame[4] = 0x00; // Cantidad alta
    frame[5] = 0x04; // Cantidad baja
    crc = modbus_crc16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;
    send_modbus_request(frame, 8);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);

    for (int i = 0; i < 3; i++) {
    bool caravana_vacia = true;
     for (int j = 0; j < 15; j++) {
        if (response[6 + (i * 25) + j] != '0') {
            caravana_vacia = false;
            break;
        }
    }
    if (caravana_vacia == false) {
        for (int j = 0; j < 15; j++) {
            animales_leidos_aux.nombre[j] = response[6 + (i * 25) + j];

        }
        animales_leidos_aux.fechaDispensado = animales_leidos_aux.fechaDispensado | ((response[6+(i*25)+16]) << 56);
        animales_leidos_aux.fechaDispensado = animales_leidos_aux.fechaDispensado | ((response[6+(i*25)+17]) << 48);
        animales_leidos_aux.fechaDispensado = animales_leidos_aux.fechaDispensado | ((response[6+(i*25)+18]) << 40);
        animales_leidos_aux.fechaDispensado = animales_leidos_aux.fechaDispensado | ((response[6+(i*25)+19]) << 32);
        animales_leidos_aux.fechaDispensado = animales_leidos_aux.fechaDispensado | ((response[6+(i*25)+20]) << 24);
        animales_leidos_aux.fechaDispensado = animales_leidos_aux.fechaDispensado | ((response[6+(i*25)+21]) << 16);
        animales_leidos_aux.fechaDispensado = animales_leidos_aux.fechaDispensado | ((response[6+(i*25)+22]) << 8);
        animales_leidos_aux.fechaDispensado = animales_leidos_aux.fechaDispensado | ((response[6+(i*25)+23]));
        animales_leidos_aux.pesoDispensado = response[6+(i*25)+24];

        if(total_animales_leidos < 100){
            animales_leidos_copia[total_animales_leidos] = animales_leidos_aux;
            total_animales_leidos++;
             }
        }
    }
    for(int i = 0; i < 3; i++){
    memset(&animales_leidos_aux, 0, sizeof(data_animal_leido));
    }
}*/
/*
void tarea82(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x82; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

         // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);   
}

void tarea83(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x83; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)
         // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);   
}

void tarea84(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x84; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

         // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);   
}

void tarea85(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x85; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

         // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);   
}

void tarea86(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x86; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

         // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);   
}

void tarea87(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x87; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

         // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);   
}

void tarea88(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x88; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

         // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);   
}

void tarea89(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x89; // Función: mandar timestamp de animales leidos para verificar cola
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)

         // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;
    crc = modbus_crc16(frame, 15);
    frame[15] = crc & 0xFF; // CRC byte bajo
    frame[16] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 17);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);   
}

*/
/*void tarea90(data_animal_leido animales_leidos_copia, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x90; // Función: escribir datos de animales leidos
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 17; // Cantidad de registros
    frame[6] = 34; // Cantidad de bytes (2 registros x 2 bytes)

    frame[7] = animales_leidos_copia[0].nombre[0]; frame[8] = animales_leidos_copia[0].nombre[1];
    frame[9] = animales_leidos_copia[0].nombre[2]; frame[10] = animales_leidos_copia[0].nombre[3];
    frame[11] = animales_leidos_copia[0].nombre[4]; frame[12] = animales_leidos_copia[0].nombre[5];
    frame[13] = animales_leidos_copia[0].nombre[6]; frame[14] = animales_leidos_copia[0].nombre[7];
    frame[15] = animales_leidos_copia[0].nombre[8]; frame[16] = animales_leidos_copia[0].nombre[9];
    frame[17] = animales_leidos_copia[0].nombre[10]; frame[18] = animales_leidos_copia[0].nombre[11];
    frame[19] = animales_leidos_copia[0].nombre[12]; frame[20] = animales_leidos_copia[0].nombre[13];
    frame[21] = animales_leidos_copia[0].nombre[14]; frame[22] = '\0';
    frame[23] = (animales_leidos_copia[0].fechaDispensado >> 56) & 0xFF; frame[24] = (animales_leidos_copia[0].fechaDispensado >> 48) & 0xFF;  // Timestamp de fecha dispensado
    frame[25] = (animales_leidos_copia[0].fechaDispensado >> 40) & 0xFF; frame[26] = (animales_leidos_copia[0].fechaDispensado >> 32) & 0xFF; 
    frame[27] = (animales_leidos_copia[0].fechaDispensado >> 24) & 0xFF; frame[28] = (animales_leidos_copia[0].fechaDispensado >> 16) & 0xFF;  
    frame[29] = (animales_leidos_copia[0].fechaDispensado >> 8) & 0xFF; frame[30] = animales_leidos_copia[0].fechaDispensado & 0xFF;  
    frame[31] = animales_leidos_copia[0].pesoDispensado; 
    crc = modbus_crc16(frame, 32);
    frame[32] = crc & 0xFF; // CRC byte bajo
    frame[33] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 34);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
}*/