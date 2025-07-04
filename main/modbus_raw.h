#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

void modbus_uart_init(void);
void modbus_send_read_holding_registers(uint8_t slave_addr, uint16_t start_addr, uint16_t quantity);
int modbus_receive_response(uint8_t *buffer, size_t len);
esp_err_t modbus_write_single_register(uint8_t slave_addr, uint16_t reg_addr, uint16_t value);
esp_err_t modbus_write_multiple_registers(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, const uint16_t *values);

