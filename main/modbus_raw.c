#include "modbus_raw.h"
#include "modbus_crc.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define UART_NUM    UART_NUM_2
#define TXD_PIN     17
#define RXD_PIN     16
#define RTS_PIN     4
#define BUF_SIZE    128

static const char *TAG = "MODBUS_RAW";

void modbus_uart_init(void) {
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

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RTS_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(RTS_PIN, 0); // recepción al inicio
}

void modbus_send_read_holding_registers(uint8_t slave_addr, uint16_t start_addr, uint16_t quantity) {
    uint8_t frame[8];
    frame[0] = slave_addr;
    frame[1] = 0x03;
    frame[2] = (start_addr >> 8) & 0xFF;
    frame[3] = start_addr & 0xFF;
    frame[4] = (quantity >> 8) & 0xFF;
    frame[5] = quantity & 0xFF;

    uint16_t crc = modbus_crc16(frame, 6);
    frame[6] = crc & 0xFF;
    frame[7] = (crc >> 8) & 0xFF;

    gpio_set_level(RTS_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(2));
    uart_write_bytes(UART_NUM, (const char *)frame, sizeof(frame));
    uart_wait_tx_done(UART_NUM, pdMS_TO_TICKS(100));
    gpio_set_level(RTS_PIN, 0);
}

int modbus_receive_response(uint8_t *buffer, size_t len) {
    return uart_read_bytes(UART_NUM, buffer, len, pdMS_TO_TICKS(500));
}

#include "modbus_crc.h"
#include "driver/uart.h"

#define MODBUS_UART_PORT UART_NUM
#define MODBUS_TIMEOUT_MS 100

esp_err_t modbus_write_single_register(uint8_t slave_addr, uint16_t reg_addr, uint16_t value)
{
    uint8_t tx_buffer[8];
    uint8_t rx_buffer[8];
    uint16_t crc;

    tx_buffer[0] = slave_addr;
    tx_buffer[1] = 0x06;  // Función 06: escribir un solo registro
    tx_buffer[2] = (reg_addr >> 8) & 0xFF;
    tx_buffer[3] = reg_addr & 0xFF;
    tx_buffer[4] = (value >> 8) & 0xFF;
    tx_buffer[5] = value & 0xFF;

    crc = modbus_crc16(tx_buffer, 6);
    tx_buffer[6] = crc & 0xFF;
    tx_buffer[7] = (crc >> 8) & 0xFF;

    uart_flush(MODBUS_UART_PORT); // Limpiar antes de enviar
    printf("Estado gpio4: %d\n", gpio_get_level(RTS_PIN));
    gpio_set_level(RTS_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(2));
    uart_write_bytes(MODBUS_UART_PORT, (const char *)tx_buffer, 8);
    uart_wait_tx_done(UART_NUM, pdMS_TO_TICKS(100));
    gpio_set_level(RTS_PIN, 0);

    // Esperar respuesta (que debe ser un eco de lo enviado)
    int len = uart_read_bytes(MODBUS_UART_PORT, rx_buffer, sizeof(rx_buffer), MODBUS_TIMEOUT_MS / portTICK_PERIOD_MS);
    ESP_LOG_BUFFER_HEX("TX", tx_buffer, 8);
    ESP_LOG_BUFFER_HEX("RX", rx_buffer, len);
    if (len != 8) return ESP_FAIL;
    
    // Verificar eco y CRC
    if (memcmp(tx_buffer, rx_buffer, 6) != 0) return ESP_FAIL;

    uint16_t crc_received = (rx_buffer[7] << 8) | rx_buffer[6];
    if (crc_received != modbus_crc16(rx_buffer, 6)) return ESP_FAIL;

    return ESP_OK;
}

esp_err_t modbus_write_multiple_registers(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, const uint16_t *values)
{
    if (quantity == 0 || quantity > 123) return ESP_ERR_INVALID_ARG;

    uint8_t tx_buffer[256];
    uint8_t rx_buffer[8];

    uint8_t byte_count = quantity * 2;

    tx_buffer[0] = slave_addr;
    tx_buffer[1] = 0x10;
    tx_buffer[2] = reg_addr >> 8;
    tx_buffer[3] = reg_addr & 0xFF;
    tx_buffer[4] = quantity >> 8;
    tx_buffer[5] = quantity & 0xFF;
    tx_buffer[6] = byte_count;

    for (int i = 0; i < quantity; ++i) {
        tx_buffer[7 + i * 2] = values[i] >> 8;
        tx_buffer[8 + i * 2] = values[i] & 0xFF;
    }

    uint16_t crc = modbus_crc16(tx_buffer, 7 + byte_count);
    tx_buffer[7 + byte_count] = crc & 0xFF;
    tx_buffer[8 + byte_count] = crc >> 8;

    size_t total_len = 9 + byte_count;

    uart_flush(UART_NUM); // Limpiar antes de enviar
    gpio_set_level(RTS_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(2));

    if (uart_write_bytes(UART_NUM, (const char *)tx_buffer, total_len) != total_len) {
        return ESP_FAIL;
    }

    uart_wait_tx_done(UART_NUM, pdMS_TO_TICKS(100));
    gpio_set_level(RTS_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    int len = uart_read_bytes(UART_NUM, rx_buffer, 8, pdMS_TO_TICKS(200));
    if (len != 8) return ESP_ERR_TIMEOUT;

    uint16_t crc_calc = modbus_crc16(rx_buffer, 6);
    uint16_t crc_recv = (rx_buffer[7] << 8) | rx_buffer[6];
    if (crc_calc != crc_recv) return ESP_ERR_INVALID_CRC;

    if (rx_buffer[0] != slave_addr || rx_buffer[1] != 0x10) return ESP_FAIL;

    return ESP_OK;
}

esp_err_t modbus_write_datos(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, const uint16_t *values)
{
    if (quantity == 0 || quantity > 123) return ESP_ERR_INVALID_ARG;

    quantity = 19;
    uint8_t tx_buffer[256];
    uint8_t rx_buffer[8];

    uint8_t byte_count = quantity * 2;

    tx_buffer[7] = 0x01;
    tx_buffer[8] = 0x02;
    tx_buffer[9] = 0x03;
    tx_buffer[10] = 0x04;
    tx_buffer[11] = 0x05;
    tx_buffer[12] = 0x06;
    tx_buffer[13] = 0x07;
    tx_buffer[14] = 0x08;
    tx_buffer[15] = 0x09;
    tx_buffer[16] = 0x0a;
    tx_buffer[17] = 0x0b;
    tx_buffer[18] = 0x0c;
    tx_buffer[19] = 0x0d;

    tx_buffer[0] = slave_addr;
    tx_buffer[1] = 0x41;
    tx_buffer[2] = reg_addr >> 8;
    tx_buffer[3] = reg_addr & 0xFF;
    tx_buffer[4] = quantity >> 8;
    tx_buffer[5] = quantity & 0xFF;
    tx_buffer[6] = byte_count;

    for (int i = 0; i < quantity; ++i) {
        tx_buffer[7 + i * 2] = values[i] >> 8;
        tx_buffer[8 + i * 2] = values[i] & 0xFF;
    }

    uint16_t crc = modbus_crc16(tx_buffer, 7 + byte_count);
    tx_buffer[7 + byte_count] = crc & 0xFF;
    tx_buffer[8 + byte_count] = crc >> 8;

    size_t total_len = 9 + byte_count;

    uart_flush(UART_NUM); // Limpiar antes de enviar
    gpio_set_level(RTS_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(2));

    if (uart_write_bytes(UART_NUM, (const char *)tx_buffer, total_len) != total_len) {
        return ESP_FAIL;
    }

    uart_wait_tx_done(UART_NUM, pdMS_TO_TICKS(100));
    gpio_set_level(RTS_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    int len = uart_read_bytes(UART_NUM, rx_buffer, 8, pdMS_TO_TICKS(200));
    if (len != 8) return ESP_ERR_TIMEOUT;

    uint16_t crc_calc = modbus_crc16(rx_buffer, 6);
    uint16_t crc_recv = (rx_buffer[7] << 8) | rx_buffer[6];
    if (crc_calc != crc_recv) return ESP_ERR_INVALID_CRC;

    if (rx_buffer[0] != slave_addr || rx_buffer[1] != 0x10) return ESP_FAIL;

    return ESP_OK;
}