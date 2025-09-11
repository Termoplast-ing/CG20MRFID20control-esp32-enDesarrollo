// 📁 reloj.c
#include <sys/time.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "structyvar.h"

#define I2C_SLAVE_ADDR 0x68
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22
#define TIMEOUT_MS 1000

esp_err_t read_time(void);

/////inicializacion de la comunicacion I2C (SDA = GPIO21)(SCL = GPIO22)/////
void init_i2c() {
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .sda_pullup_en = true,
        .scl_pullup_en = true,
        .master.clk_speed = 100000,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    vTaskDelay(pdMS_TO_TICKS(100));
    
    struct timeval now = { .tv_sec = RTC_time, .tv_usec = 0 };
    settimeofday(&now, NULL);
read_time(); // Leer la hora inicial del RTC
}

uint8_t convertir_a_bcd(uint8_t numero) {
    if (numero > 99) {
        return 0xFF; // Valor de error
    }

    uint8_t decenas = numero / 10;
    uint8_t unidades = numero % 10;


    return (decenas << 4) | unidades;
}

/////funcion para leer los registros del ds1307/////
esp_err_t ds1307_read_register(uint8_t reg_addr, uint8_t *data) {
    esp_err_t err = i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, &reg_addr, 1, pdMS_TO_TICKS(TIMEOUT_MS));
    if (err != ESP_OK) return err;
    return i2c_master_read_from_device(I2C_NUM_0, I2C_SLAVE_ADDR, data, 1, pdMS_TO_TICKS(TIMEOUT_MS));
}

/////funcion para escribir en los registros del ds1307///// 
esp_err_t ds1307_write_register(uint8_t reg_addr, uint8_t data) {
    uint8_t write_buffer[2] = { reg_addr, data };
    
    return i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, write_buffer, sizeof(write_buffer), pdMS_TO_TICKS(TIMEOUT_MS));
}

/////funcion para de BDC a decimal/////
uint8_t bcd_to_decimal(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/////funcion para leer fecha y hora en tiempo real con tratamiento de errores/////
esp_err_t read_time() {
    uint8_t seconds, minutes, hours,day,month,year;
    int retries = 3;
    esp_err_t result;

    while (retries--) {
        result = ds1307_read_register(0x00, &seconds);
        result |= ds1307_read_register(0x01, &minutes);
        result |= ds1307_read_register(0x02, &hours);
        result |= ds1307_read_register(0x04, &day);
        result |= ds1307_read_register(0x05, &month);
        result |= ds1307_read_register(0x06, &year);
        if (result == ESP_OK) break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    printf("reloj del READ %02X:%02X:%02X - %02X/%02X/%02X\n", seconds, minutes, hours, day, month, year);
    if (result != ESP_OK) {
        printf("Error al leer la hora del DS1307 tras múltiples intentos\n");
        return ESP_FAIL;
    }
    ///si se logra leer la hora se limpia el error y se guarda en la estructura de tiempo
    ///y lo paso a un formato de tiempo
    RTC_hora.tm_hour = bcd_to_decimal(hours);
    RTC_hora.tm_min = bcd_to_decimal(minutes);
    RTC_hora.tm_sec = bcd_to_decimal(seconds);
    RTC_hora.tm_mday = bcd_to_decimal(day);
    RTC_hora.tm_mon = bcd_to_decimal(month) - 1;
    RTC_hora.tm_year = bcd_to_decimal(year) + 100;
    RTC_time = mktime(&RTC_hora);
    printf("time RTC_time READ: %lld\n", RTC_time);
    return ESP_OK;

    
}

void actualizar_reloj(time_t timestamp) {
    struct timeval now = { .tv_sec = timestamp, .tv_usec = 0 };
    settimeofday(&now, NULL);
    ESP_LOGI("RELOJ", "Reloj actualizado a %lld", (long long)timestamp);
       // RTC_time = tiempo; // Guardar el tiempo actual en la variable global
    

    localtime_r(&timestamp, &RTC_hora); // Convertir el tiempo a la estructura localtime
    RTC_time= timestamp;

    printf("Hora actual: %02d:%02d:%02d\n", RTC_hora.tm_hour, RTC_hora.tm_min, RTC_hora.tm_sec);
    printf("Fecha actual: %02d/%02d/%04d\n", RTC_hora.tm_mday, RTC_hora.tm_mon + 1, RTC_hora.tm_year + 1900);

    RTC_hora.tm_sec=convertir_a_bcd(RTC_hora.tm_sec);
    ds1307_write_register(0x00, RTC_hora.tm_sec); // segundos
    RTC_hora.tm_min=convertir_a_bcd(RTC_hora.tm_min);
    ds1307_write_register(0x01, RTC_hora.tm_min); // minutos
    RTC_hora.tm_hour=convertir_a_bcd(RTC_hora.tm_hour);
    ds1307_write_register(0x02, RTC_hora.tm_hour); // horas
    RTC_hora.tm_mday =convertir_a_bcd(RTC_hora.tm_mday);
    ds1307_write_register(0x04, RTC_hora.tm_mday); // día de la semana (1-7)
    RTC_hora.tm_mon=convertir_a_bcd(RTC_hora.tm_mon+1);
    ds1307_write_register(0x05, RTC_hora.tm_mon); // mes (1-12)
    printf("Año antes de escribir: %d\n", RTC_hora.tm_year);
    RTC_hora.tm_year=convertir_a_bcd(RTC_hora.tm_year-100);
    ds1307_write_register(0x06, RTC_hora.tm_year); // año (a partir de 2000)
    
    ds1307_write_register(0x07,0x93);

    actualizarRTC = true;


}


