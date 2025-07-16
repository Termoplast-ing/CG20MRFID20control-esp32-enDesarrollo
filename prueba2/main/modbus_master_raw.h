#pragma once
#include <stdint.h>
#include "driver/uart.h"
#include "structyvar.h"


void modbus_master_init(uart_port_t uart_num);
void tarea51(data_animal a, time_t t);