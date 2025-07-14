/*
 * Maestro Modbus RTU para ESP32 (ESP-IDF v5.4) - Versión Simplificada
 * Configuración completa en código - UART2/GPIO4 - 9600 8N1
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
#define SLAVE_ADDR 1      // Dirección del esclavo
#define UART_NUM   UART_NUM_2
#define BAUD_RATE  9600
#define RTS_PIN    4      // GPIO para control RS485
#define TX_PIN     17     // GPIO para TX
#define RX_PIN     16     // GPIO para RX
#define REG_START  0      // Dirección inicial
#define REG_COUNT  4      // Número de registros
#define READ_HOLDING_REG 0x03  // Código función lectura registros holding

void app_main(void)
{
    // 1. Configuración UART (solo parámetros, el driver lo instala Modbus)
    ESP_LOGI(TAG, "Configurando parámetros UART...");
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };
    
    // Configurar UART sin control RTS automático
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, 
                TX_PIN,    // GPIO17 - TX
                RX_PIN,    // GPIO16 - RX
                UART_PIN_NO_CHANGE, // No usar RTS automático
                UART_PIN_NO_CHANGE)); // CTS no usado

    // Configurar pin RTS manualmente como GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RTS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(RTS_PIN, 0)); // Inicialmente en bajo

    // Verificar estado del pin RTS
    ESP_LOGI(TAG, "Estado inicial RTS: %d", gpio_get_level(RTS_PIN));

    // 2. Inicialización Modbus
    void* master_handler = NULL;
    mb_communication_info_t comm = {
        .mode = MB_MODE_RTU,
        .port = UART_NUM,
        .baudrate = BAUD_RATE,
        .parity = MB_PARITY_NONE
    };

    ESP_ERROR_CHECK(mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler));
    ESP_ERROR_CHECK(mbc_master_setup(&comm));
    ESP_ERROR_CHECK(mbc_master_start());
    
    ESP_LOGI(TAG, "Maestro Modbus inicializado");
    ESP_LOGI(TAG, "UART%d, TX:%d, RX:%d, RTS:%d, Baud:%d", 
             UART_NUM, TX_PIN, RX_PIN, RTS_PIN, BAUD_RATE);

    // 3. Bucle principal de lectura
    float reg_values[REG_COUNT] = {0};
    mb_param_request_t request = {
        .slave_addr = SLAVE_ADDR,
        .command = READ_HOLDING_REG,  // Usamos código de función directo
        .reg_start = REG_START,
        .reg_size = REG_COUNT * 2  // Cada float ocupa 2 registros
    };

    while(1) {
        // Activar transmisión (RTS alto)
        ESP_ERROR_CHECK(gpio_set_level(RTS_PIN, 1));
        vTaskDelay(pdMS_TO_TICKS(1)); // Pequeño retardo para estabilizar

        // Enviar consulta Modbus
        esp_err_t err = mbc_master_send_request(&request, (void*)reg_values);

        // Desactivar transmisión (RTS bajo)
        ESP_ERROR_CHECK(gpio_set_level(RTS_PIN, 0));

        if(err == ESP_OK) {
            ESP_LOGI(TAG, "Registros leídos:");
            ESP_LOGI(TAG, "0: %.2f, 1: %.2f, 2: %.2f, 3: %.2f", 
                    reg_values[0], reg_values[1], 
                    reg_values[2], reg_values[3]);
        } else {
            ESP_LOGE(TAG, "Error lectura: 0x%x", err);
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Esperar 2 segundos
    }
}
 */
/*
 #include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "uart_modbus.h"

#define TAG "APP_MAIN"

void app_main(void)
{
    uart_modbus_init();

    uint8_t frame[8];
    frame[0] = 0x05;        // Dirección esclavo
    frame[1] = 0x03;        // Código de función: Leer Holding Register
    frame[2] = 0x00;        // Dirección inicial alta
    frame[3] = 0x00;        // Dirección inicial baja
    frame[4] = 0x00;        // Cantidad de registros alta
    frame[5] = 0x01;        // Cantidad de registros baja

    ESP_LOGI(TAG, "⏩ Enviando solicitud de lectura de registro...");
    int send_len = uart_modbus_send(frame, 6);
    ESP_LOGI(TAG, "📤 Enviados %d bytes", send_len);

    uint8_t response[256];
    int len = uart_modbus_receive(response, sizeof(response), 500);
    if (len > 0) {
        ESP_LOGI(TAG, "✅ Respuesta recibida (%d bytes):", len);
        for (int i = 0; i < len; i++) {
            printf("%02X ", response[i]);
        }
        printf("\n");
    } else {
        ESP_LOGW(TAG, "⚠️ No se recibió respuesta válida (%d)", len);
    }
}*/



