// 📁 main.h
#ifndef STRUCTYVAR_H
#define STRUCTYVAR_H


#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h" 
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "freertos/semphr.h"

void inicializar_animales_actual_nombre(void);
void inicializar_config(void);
void inicializar_curvas(void);

typedef struct {
    int id_tolva;
    bool motor_on;
    char ultimo_json[256];
} ConfigTolva;

typedef struct {
    bool errorHoraDS1307;
    bool errorUART;
    bool tolvaVacia;
    bool jaulaTrabada;
    bool errorMotor;
    bool errorAgua;
    bool caravanaNoReconocida;
} error_status_t;

typedef struct {
    char nombre[16];
    uint8_t tipoCurva;
    uint8_t pesoDosis;
    time_t fechaServicio;
    uint8_t indiceCorporal;
    bool agua;
    uint8_t cantDosis;
    uint16_t intervaloMin;    
} data_animal;

/////Estructura para almacenar los datos de los animales leidos por la antena RFID/////
typedef struct{
    char nombre[16];
    time_t fechaDispensado;
    uint8_t pesoDispensado;
    uint8_t nroTolva;  
} data_animal_leido;

/////Estructura para almacenar los segmentos de las curvas de crecimiento/////
typedef struct{
    uint8_t inicio;
    uint8_t pesoInicio;
} segmento;

/////Estructura para almacenar las curvas de crecimiento/////
typedef struct{
    segmento segmentos[17];  
} tipo_curva;

typedef struct {
    uint8_t caravanaLibre1[16];
    uint8_t caravanaLibre2[16];
    uint8_t caravanaLibre3[16];
    uint8_t caravanaLibre4[16];
    uint8_t caravanaLibre5[16];
    uint8_t calibracionMotor;
    uint8_t calibracionAgua;
    uint8_t pesoAnimalDesconocido;
} configuration;

extern uint8_t indice;
extern data_animal animales_actual[20];
extern SemaphoreHandle_t mutex_animales;
extern data_animal animales_copia[20];
extern data_animal_leido animales_leidos_actual[100];
extern SemaphoreHandle_t mutex_animales_leidos;
extern data_animal_leido animales_leidos_copia[100];
extern tipo_curva curvas_actual[5];
extern SemaphoreHandle_t mutex_curvas;
extern tipo_curva curvas_copia[5];
extern configuration configuracion_actual;
extern SemaphoreHandle_t mutex_configuracion;
extern configuration configuracion_copia;

extern time_t timestamp_animales;
extern SemaphoreHandle_t mutex_Tanimales;
extern time_t copia_Tanimales;
extern time_t timestamp_animales_leidos;
extern SemaphoreHandle_t mutex_Tanimales_leidos;
extern time_t copia_Tanimales_leidos;
extern time_t timestamp_curvas;
extern SemaphoreHandle_t mutex_Tcurvas;
extern time_t copia_Tcurvas;
extern time_t timestamp_configuracion;
extern SemaphoreHandle_t mutex_Tconfiguracion;
extern time_t copia_Tconfiguracion;
extern time_t timestamp_relojRTC;

//extern uint8_t response[9];

extern ConfigTolva config_actual;
extern SemaphoreHandle_t mutex_config;
extern uint8_t timeOK;
extern data_animal_leido animal_leido_AUX;
extern time_t RTC_time;
extern bool envio_RTC;
extern struct tm RTC_hora;
extern time_t RTC_time;
extern bool actualizarRTC;
extern char respuesta[2];

#endif // STRUCTYVAR_H
