/**
 * Maestro Modbus RTU para ESP32 (ESP-IDF v5.4) con control automático de RTS (RS485 Half Duplex)
 * UART2: TX=GPIO17, RX=GPIO16, RTS=GPIO4 (automático)
 * Protocolo: 9600 8N1
 */
/*
 #include <stdio.h>
 #include "esp_log.h"
 #include "driver/gpio.h"
 #include "mbcontroller.h"
 #include "sdkconfig.h"
 #include "freertos/FreeRTOS.h"
 #include "freertos/task.h"
 
 #define TAG "MODBUS_MASTER"
 #define SLAVE_ADDR     5
 #define UART_NUM       UART_NUM_2
 #define BAUD_RATE      9600
 #define TXD_PIN        17
 #define RXD_PIN        16
 #define RTS_PIN        4
 #define REG_START      0
 #define REG_COUNT      1
 #define READ_HOLDING_REG 0x03
 
 void app_main(void) {
     void* master_handler = NULL;
 
     // Configurar parámetros de comunicación Modbus
     mb_communication_info_t comm = {
         .mode = MB_MODE_RTU,
         .port = UART_NUM,
         .baudrate = BAUD_RATE,
         .parity = MB_PARITY_NONE
     };
 
     // Iniciar y configurar el maestro Modbus
     ESP_ERROR_CHECK(mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler));
     ESP_ERROR_CHECK(mbc_master_setup(&comm));
     ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, RTS_PIN, UART_PIN_NO_CHANGE)); // RTS automático
     ESP_ERROR_CHECK(mbc_master_start());
 
     // Esperar un poco para que el esclavo esté listo
     vTaskDelay(pdMS_TO_TICKS(1000));
 
     while (1) {
         uint16_t reg_value = 0;
         mb_param_request_t request = {
             .slave_addr = SLAVE_ADDR,
             .command = READ_HOLDING_REG,
             .reg_start = REG_START,
             .reg_size = REG_COUNT
         };
 
         esp_err_t err = mbc_master_send_request(&request, (void*)&reg_value);
         if (err == ESP_OK) {
             ESP_LOGI(TAG, "✅ Esclavo [%d] responde - Valor recibido: 0x%04X", SLAVE_ADDR, reg_value);
         } else {
             ESP_LOGE(TAG, "⛔ Error comunicación con esclavo [%d]: 0x%x", SLAVE_ADDR, err);
         }
 
         vTaskDelay(pdMS_TO_TICKS(2000));
     }
 }*/



//FUNCIONA BIEN ES TODO MANUAL NO UZA LA LIBRERIA DE MODBUS

/*
 #include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define UART_NUM    UART_NUM_2
#define TXD_PIN     17
#define RXD_PIN     16
#define RTS_PIN     4
#define BUF_SIZE    128

#define TAG "MODBUS_RAW"

// Función para calcular CRC16 Modbus
uint16_t calculate_crc(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
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

// Construye trama Modbus dinámicamente
void build_modbus_frame(uint8_t *frame, uint8_t slave_addr, uint8_t function, 
                       uint16_t reg_addr, uint16_t value) {
    frame[0] = slave_addr;
    frame[1] = function;
    frame[2] = (reg_addr >> 8) & 0xFF;
    frame[3] = reg_addr & 0xFF;
    
    if(function == 0x03) { // Lectura
        frame[4] = (value >> 8) & 0xFF; // Cantidad de registros (high byte)
        frame[5] = value & 0xFF;        // Cantidad de registros (low byte)
    } else { // Escritura (0x06)
        frame[4] = (value >> 8) & 0xFF; // Valor a escribir (high byte)
        frame[5] = value & 0xFF;        // Valor a escribir (low byte)
    }
    
    uint16_t crc = calculate_crc(frame, 6);
    frame[6] = crc & 0xFF;  // CRC LSB
    frame[7] = crc >> 8;    // CRC MSB
}

void app_main(void)
{
    // Configuración UART
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Configuración RTS manual
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RTS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(RTS_PIN, 0); // Iniciar en modo recepción

    uint8_t response[BUF_SIZE];

    while (1) {
        uint8_t frame[8];
        static uint16_t write_value = 0;
        
        // Alternar entre lectura y escritura
        static bool write_operation = false;
        
        if(write_operation) {
            // Operación de escritura
            build_modbus_frame(frame, 0x01, 0x06, 0x0000, write_value);
            ESP_LOGI(TAG, "📝 Escribiendo valor 0x%04X en registro 0", write_value);
            write_value = (write_value + 1) % 100; // Incrementar valor para próxima escritura
        } else {
            // Operación de lectura
            build_modbus_frame(frame, 0x01, 0x03, 0x0000, 0x0001);
            ESP_LOGI(TAG, "📖 Leyendo registro 0");
        }

        gpio_set_level(RTS_PIN, 1); // Activar transmisión
        vTaskDelay(pdMS_TO_TICKS(2));
        
        uart_write_bytes(UART_NUM, (const char *)frame, sizeof(frame));
        uart_wait_tx_done(UART_NUM, pdMS_TO_TICKS(100));
        
        gpio_set_level(RTS_PIN, 0); // Volver a recepción

        int len = uart_read_bytes(UART_NUM, response, BUF_SIZE, pdMS_TO_TICKS(500));
        if (len > 0) {
            ESP_LOGI(TAG, "✅ Respuesta (%d bytes):", len);
            for (int i = 0; i < len; i++) {
                printf("%02X ", response[i]);
            }
            printf("\n");
            
            // Verificar CRC de respuesta
            uint16_t recv_crc = (response[len-1] << 8) | response[len-2];
            uint16_t calc_crc = calculate_crc(response, len-2);
            if(recv_crc != calc_crc) {
                ESP_LOGE(TAG, "❌ Error CRC: esperado 0x%04X, recibido 0x%04X", calc_crc, recv_crc);
            }
        } else {
            ESP_LOGW(TAG, "⚠️ No se recibió respuesta");
        }

        write_operation = !write_operation; // Alternar operación
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}*/

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"

#define UART_NUM    UART_NUM_2
#define TXD_PIN     17
#define RXD_PIN     16
#define RTS_PIN     4
#define BUF_SIZE    128

#define TAG "MODBUS_RAW"

static const uint8_t request_frame[] = {
    0x05,       // Slave address
    0x03,       // Function code: Read Holding Register
    0x00, 0x00, // Start address: 0
    0x00, 0x01, // Quantity: 1
    0x85, 0xDB  // CRC (LSB first)
};

void app_main(void)
{
    // Configuración UART
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Configuración RTS manual
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RTS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(RTS_PIN, 0); // Iniciar en modo recepción

    uint8_t response[BUF_SIZE];

    while (1) {
        ESP_LOGI(TAG, "⏩ Enviando solicitud Modbus...");

        gpio_set_level(RTS_PIN, 1); // Activar transmisión
        vTaskDelay(pdMS_TO_TICKS(2)); // Pequeño delay para estabilizar

        uart_write_bytes(UART_NUM, (const char *)request_frame, sizeof(request_frame));
        uart_wait_tx_done(UART_NUM, pdMS_TO_TICKS(100));

        gpio_set_level(RTS_PIN, 0); // Volver a recepción

        int len = uart_read_bytes(UART_NUM, response, BUF_SIZE, pdMS_TO_TICKS(500));
        if (len > 0) {
            ESP_LOGI(TAG, "✅ Respuesta recibida (%d bytes):", len);
            for (int i = 0; i < len; i++) {
                printf("%02X ", response[i]);
            }
            printf("\n");
        } else {
            ESP_LOGW(TAG, "⚠️  No se recibió respuesta");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}