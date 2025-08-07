#ifndef RELOJ_H
#define RELOJ_H

#include <time.h>

void actualizar_reloj(time_t timestamp);
void init_i2c() ;
esp_err_t ds1307_read_register(uint8_t reg_addr, uint8_t *data) ;
esp_err_t ds1307_write_register(uint8_t reg_addr, uint8_t data);
uint8_t bcd_to_decimal(uint8_t bcd) ;
esp_err_t read_time(void);
uint8_t convertir_a_bcd(uint8_t numero);

#endif