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

//static const char *TAG = "MODBUS_MASTER";

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
    uint16_t crc;

    // --- Ejemplo: Leer registros (0x03) ---
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
    printf("delay\n");
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

    
void tarea51(data_animal animal, time_t tiempo) {
    uint8_t frame[BUF_SIZE];
    uint8_t response[BUF_SIZE];
    uint16_t crc;

    // --- Ejemplo: Escribir un registro (0x06) ---
    frame[0] = SLAVE_ADDR;
    frame[1] = 0x51; // Función: escribir datos de dieta de caravanas
    frame[2] = 0x00;
    frame[3] = 0x02; // Dirección inicial
    frame[4] = 0x00;
    frame[5] = 25; // Cantidad de registros
    frame[6] = 49; // Cantidad de bytes (2 registros x 2 bytes)
    
    // String de número caravana
    //snprintf((char *)&frame[17], sizeof(frame) - 17, "%s", animal.nombre);

        // Timestamp de central
    frame[7] = (tiempo >> 56) & 0xFF;
    frame[8] = (tiempo >> 48) & 0xFF;
    frame[9] = (tiempo >> 40) & 0xFF;
    frame[10] = (tiempo >> 32) & 0xFF;
    frame[11] = (tiempo >> 24) & 0xFF;
    frame[12] = (tiempo >> 16) & 0xFF;
    frame[13] = (tiempo >> 8) & 0xFF;
    frame[14] = tiempo & 0xFF;

    frame[15] = animal.nombre[0]; frame[16] = animal.nombre[1];   // string de numero caravana
    frame[17] = animal.nombre[2]; frame[18] = animal.nombre[3];   // pasado por caracter 
    frame[19] = animal.nombre[4]; frame[20] = animal.nombre[5];   // de a un byte
    frame[21] = animal.nombre[6]; frame[22] = animal.nombre[7];   // 
    frame[23] = animal.nombre[8]; frame[24] = animal.nombre[9];   // tamaño 16 bytes
    frame[25] = animal.nombre[10]; frame[26] = animal.nombre[11];   // 
    frame[27] = animal.nombre[12]; frame[28] = animal.nombre[13];   // desde dir. 15
    frame[29] = animal.nombre[14]; frame[30] = '\0';  // hasta dir. 30

    frame[31] = animal.tipoCurva; frame[32] = animal.pesoDosis;  // dir. 31 = tipo de curva // dir. 32 = peso dosis

    frame[33] = (tiempo >> 56) & 0xFF; frame[34] = (tiempo >> 48) & 0xFF;  // Timestamp de fecha Inseminacion
    frame[35] = (tiempo >> 40) & 0xFF; frame[36] = (tiempo >> 32) & 0xFF;  // tamaño 8 bytes
    frame[37] = (tiempo >> 24) & 0xFF; frame[38] = (tiempo >> 16) & 0xFF;  // desde dir. 33
    frame[39] = (tiempo >> 8) & 0xFF; frame[40] = tiempo & 0xFF;  // hasta dir. 40

    frame[41] = animal.indiceCorporal; frame[42] = animal.agua;  // dir. 41 = indice corporal // dir. 42 = booleano del agua
    frame[43] = animal.cantDosis; frame[44] = ((animal.intervaloMin >> 8) & 0xFF);  // dir. 43 = cantidad dosis // dir. 44 = intervalomin.(byte alto)
    frame[45] = (animal.intervaloMin & 0xFF); frame[46] = 0x00;    // dir. 44 = intervalomin.(byte bajo)

    crc = modbus_crc16(frame, 47);
    frame[47] = crc & 0xFF; // CRC byte bajo
    frame[48] = crc >> 8;   // CRC byte alto
    send_modbus_request(frame, 49);
    printf("ya\n");
    vTaskDelay(pdMS_TO_TICKS(200));
    receive_modbus_response(response, BUF_SIZE);
}