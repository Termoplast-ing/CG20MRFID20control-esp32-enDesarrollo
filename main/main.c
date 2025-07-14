



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


// Define CONFIG_FREERTOS_HZ if not defined (default 100 Hz)
#ifndef CONFIG_FREERTOS_HZ
#define CONFIG_FREERTOS_HZ 100
#endif
#define TIMEOUT_MS 1000

#define UART_NUM2 UART_NUM_2
#define BUF_SIZE 1024
#define TXD_PIN 17
#define RXD_PIN 16
//#define GPIO_NUM_4 4 // Pin GPIO para RTS
#define SLAVE_ADDR 0x05

static const char *TAG = "MODBUS_MASTER";

uint16_t modbus_crc16(const uint8_t *buf, int len);

void send_modbus_request(const uint8_t *frame, int len) {
    printf("chau");
    printf("%d", len);
    for(int i = 0; i < len; i++) {
        printf("%d ", frame[i]);
    }
    //printf("\n");
    gpio_set_level(GPIO_NUM_4, 1);
    vTaskDelay(pdMS_TO_TICKS(2));
    uart_write_bytes(UART_NUM2, (const char *)frame, len);
    uart_wait_tx_done(UART_NUM2, pdMS_TO_TICKS(100));
    gpio_set_level(GPIO_NUM_4, 0);
}

int receive_modbus_response(uint8_t *buf, int maxlen) {

    int len = uart_read_bytes(UART_NUM2, buf, maxlen, pdMS_TO_TICKS(200));
    if (len > 0) {
        printf("Respuesta recibida (%d bytes):\n", len);
        for (int i = 0; i < len; i++) printf("%02X ", buf[i]);
        printf("\n");
    }
    return len;
}

void modbus_master_task(void *arg) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    int len;
    uint16_t crc;

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

    // --- Ejemplo: Escribir un registro (0x06) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x06;
    frame[2] = 0x00;
    frame[3] = 0x01; // Dirección del registro
    frame[4] = 0x12;
    frame[5] = 0x34; // Valor a escribir
    crc = modbus_crc16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = crc >> 8;
    send_modbus_request(frame, 8);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);

    // --- Ejemplo: Escribir múltiples registros (0x10) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x10;
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 0x02; // Cantidad de registros
    frame[6] = 0x04; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 0x00; frame[8] = 0xAA; // Valor 1
    frame[9] = 0x00; frame[10] = 0xBB; // Valor 2
    crc = modbus_crc16(frame, 11);
    frame[11] = crc & 0xFF;
    frame[12] = crc >> 8;
    send_modbus_request(frame, 13);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);

            // --- Ejemplo: Escribir múltiples registros (0x40) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x40; // Función: escribir datos de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 10; // Cantidad de registros
    frame[6] = 20; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 0x01; frame[8] = 0x02;   // Timestamp de central   
    frame[9] = 0x03; frame[10] = 0x04;  // tamaño 8 bytes
    frame[11] = 0x05; frame[12] = 0x06; // desde dir. 07
    frame[13] = 0x07; frame[14] = 0x08; // hasta dir. 14
    frame[15] = 0x10; frame[16] = 0x11;  // dir. 95 = calibracion peso // dir. 95 = calibracion agua
    frame[17] = 0x12; frame[18] =0x00;   // dir. 97 = peso dosis desconocido

    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    printf("ahora si que si\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    printf("delay\n");

    // --- Ejemplo: Escribir múltiples registros (0x41) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x41; // Función: escribir datos de caravana libre 1
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 12; // Cantidad de registros
    frame[6] = 24; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 'h'; frame[8] = 'o';   // string de numero caravana libre 1
    frame[9] = 'l'; frame[10] = 'a';   // pasado por caracter 
    frame[11] = 'm'; frame[12] = 'u';   // de a un byte
    frame[13] = 'n'; frame[14] = 'd';   // 
    frame[15] = 'o'; frame[16] = '1';   // tamaño 16 bytes
    frame[17] = '1'; frame[18] = '1';   // 
    frame[19] = '1'; frame[20] = '1';   // desde dir. 15
    frame[21] = '1'; frame[22] = '\0';  // hasta dir. 30
    
    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    printf("ahora si que si\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    printf("delay\n");
      
    // --- Ejemplo: Escribir múltiples registros (0x42) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x42; // Función: escribir datos de caravana libre 2
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 12; // Cantidad de registros
    frame[6] = 24; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 'h'; frame[8] = 'o';   // string de numero caravana libre 2
    frame[9] = 'l'; frame[10] = 'a';   // pasado por caracter 
    frame[11] = 'm'; frame[12] = 'u';   // de a un byte
    frame[13] = 'n'; frame[14] = 'd';   // 
    frame[15] = 'o'; frame[16] = '2';   // tamaño 16 bytes
    frame[17] = '2'; frame[18] = '2';   // 
    frame[19] = '2'; frame[20] = '2';   // desde dir. 31
    frame[21] = '2'; frame[22] = '\0';  // hasta dir. 46

    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    printf("ahora si que si\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    printf("delay\n");
    
/*    // --- Ejemplo: Escribir múltiples registros (0x43) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x43; // Función: escribir datos de caravana libre 3
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 10; // Cantidad de registros
    frame[6] = 20; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 'h'; frame[8] = 'o';   // string de numero caravana libre 3
    frame[9] = 'l'; frame[10] = 'a';   // pasado por caracter 
    frame[11] = 'm'; frame[12] = 'u';   // de a un byte
    frame[13] = 'n'; frame[14] = 'd';   // 
    frame[15] = 'o'; frame[16] = '3';   // tamaño 16 bytes
    frame[17] = '3'; frame[18] = '3';   // 
    frame[19] = '3'; frame[20] = '3';   // desde dir. 47
    frame[21] = '3'; frame[22] = '\0';  // hasta dir. 63

    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    printf("ahora si que si\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    printf("delay\n");

    // --- Ejemplo: Escribir múltiples registros (0x44) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x44; // Función: escribir datos de caravana libre 4
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 10; // Cantidad de registros
    frame[6] = 20; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 'h'; frame[8] = 'o';   // string de numero caravana 4
    frame[9] = 'l'; frame[10] = 'a';   // pasado por caracter 
    frame[11] = 'm'; frame[12] = 'u';   // de a un byte
    frame[13] = 'n'; frame[14] = 'd';   // 
    frame[15] = 'o'; frame[16] = '4';   // tamaño 16 bytes
    frame[17] = '4'; frame[18] = '4';   // 
    frame[19] = '4'; frame[20] = '4';   // desde dir. 63
    frame[21] = '4'; frame[22] = '\0';  // hasta dir. 78

    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    printf("ahora si que si\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    printf("delay\n");
    
    // --- Ejemplo: Escribir múltiples registros (0x45) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x45; // Función: escribir datos de caravana libre 5
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 10; // Cantidad de registros
    frame[6] = 20; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 'h'; frame[8] = 'o';   // string de numero caravana 5
    frame[9] = 'l'; frame[10] = 'a';   // pasado por caracter 
    frame[11] = 'm'; frame[12] = 'u';   // de a un byte
    frame[13] = 'n'; frame[14] = 'd';   // 
    frame[15] = 'o'; frame[16] = '5';   // tamaño 16 bytes
    frame[17] = '5'; frame[18] = '5';   // 
    frame[19] = '5'; frame[20] = '5';   // desde dir. 79
    frame[21] = '5'; frame[22] = '\0';  // hasta dir. 94
   
    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    printf("ahora si que si\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    printf("delay\n");
*/
       // --- Ejemplo: Escribir múltiples registros (0x51) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x51; // Función: escribir datos de dieta de caravanas
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 25; // Cantidad de registros
    frame[6] = 49; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 0x11; frame[8] = 0x02;   // Timestamp de central   
    frame[9] = 0x03; frame[10] = 0x04;  // tamaño 8 bytes
    frame[11] = 0x05; frame[12] = 0x06; // desde dir. 07
    frame[13] = 0x07; frame[14] = 0x08; // hasta dir. 14

    frame[15] = 'h'; frame[16] = 'o';   // string de numero caravana
    frame[17] = 'l'; frame[18] = 'a';   // pasado por caracter 
    frame[19] = 'm'; frame[20] = 'u';   // de a un byte
    frame[21] = 'n'; frame[22] = 'd';   // 
    frame[23] = 'o'; frame[24] = 'c';   // tamaño 16 bytes
    frame[25] = 'g'; frame[26] = '2';   // 
    frame[27] = '0'; frame[28] = 'm';   // desde dir. 15
    frame[29] = 'r'; frame[30] = '\0';  // hasta dir. 30

    frame[31] = 0x10; frame[32] = 0x11;  // dir. 31 = tipo de curva // dir. 32 = peso dosis

    frame[33] = 0x12; frame[34] = 0x13;  // Timestamp de fecha Inseminacion
    frame[35] = 0x14; frame[36] = 0x15;  // tamaño 8 bytes
    frame[37] = 0x16; frame[38] = 0x17;  // desde dir. 33
    frame[39] = 0x18; frame[40] = 0x19;  // hasta dir. 40

    frame[41] = 0x20; frame[42] = 0x21;  // dir. 41 = indice corporal // dir. 42 = booleano del agua
    frame[43] = 0x22; frame[44] = 0x23;  // dir. 43 = cantidad dosis // dir. 44 = intervalomin.(byte alto)
    frame[45] = 0x24; frame[46] = 0x00;    // dir. 44 = intervalomin.(byte bajo)

    crc = modbus_crc16(frame, 47);
    frame[47] = crc & 0xFF; // CRC byte bajo
    frame[48] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 49);
    printf("ya\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);


    // --- Ejemplo: Escribir múltiples registros (0x60) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x60; // Función: escribir datos de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 9; // Cantidad de registros
    frame[6] = 17; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 0x01; frame[8] = 0x02;   // Timestamp de central   
    frame[9] = 0x03; frame[10] = 0x04;  // tamaño 8 bytes
    frame[11] = 0x05; frame[12] = 0x06; // desde dir. 07
    frame[13] = 0x07; frame[14] = 0x08; // hasta dir. 14
    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    

    
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x61; // Función: escribir datos de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 39; // Cantidad de registros
    frame[6] = 78; // Cantidad de bytes (2 registros x 2 bytes)
    for(uint8_t i=0; i < 17; i++) {
        frame[7+(i*2)] = 10 + (i*2); // dir. 15 a 31
        frame[8+(i*2)] = 11 + ((i*2)); // valor de configuracion
        frame[41+(i*2)] = 44 + (i*2); // dir. 31 a 48
        frame[42+(i*2)] = 45 + ((i*2)); // valor de configuracion
    }

    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    

    frame[0] = SLAVE_ADDR;
    frame[1] = 0x70; // Función: escribir datos de configuracion
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 8; // Cantidad de registros
    frame[6] = 16; // Cantidad de bytes (2 registros x 2 bytes)
    frame[7] = 0x01; frame[8] = 0x02;   // Timestamp de central   
    frame[9] = 0x03; frame[10] = 0x04;  // tamaño 8 bytes
    frame[11] = 0x05; frame[12] = 0x06; // desde dir. 07
    frame[13] = 0x07; frame[14] = 0x08; // hasta dir. 14

    crc = modbus_crc16(frame, frame[6]-2);
    frame[frame[6]-2] = crc & 0xFF; // CRC byte bajo
    frame[frame[6]-1] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame,frame[6]);
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
    
    vTaskDelete(NULL);
}

void app_main(void) {

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