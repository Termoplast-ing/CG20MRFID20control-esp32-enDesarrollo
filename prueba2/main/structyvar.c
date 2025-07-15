#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h" 
#include "structyvar.h"
#include "time.h"

// === CONFIGURACIÓN ===
configuration configuracion_actual = {0};
configuration configuracion_copia = {0};
SemaphoreHandle_t mutex_configuracion = NULL;

// === ANIMALES ===
data_animal animales_actual[20] = {0};
data_animal animales_copia[20] = {0};
SemaphoreHandle_t mutex_animales = NULL;

// Inicialización de nombre para animales_actual[0]
void inicializar_animales_actual_nombre() {
	animales_actual[0].nombre[0] = '9';
	animales_actual[0].nombre[1] = '8';
	animales_actual[0].nombre[2] = '2';
	animales_actual[0].nombre[3] = '0';
	animales_actual[0].nombre[4] = '0';
	animales_actual[0].nombre[5] = '0';
	animales_actual[0].nombre[6] = '4';
	animales_actual[0].nombre[7] = '5';
	animales_actual[0].nombre[8] = '9';
	animales_actual[0].nombre[9] = '5';
	animales_actual[0].nombre[10] = '7';
	animales_actual[0].nombre[11] = '8';
	animales_actual[0].nombre[12] = '9';
	animales_actual[0].nombre[13] = '1';
	animales_actual[0].nombre[14] = '8';
	animales_actual[0].nombre[15] = '\0'; // Asegurar el fin de cadena
    animales_actual[0].tipoCurva = 1; // Asignar un tipo de curva
    animales_actual[0].pesoDosis = 10; // Asignar un peso
    animales_actual[0].fechaServicio = time(NULL); // Asignar fecha de servicio
    animales_actual[0].indiceCorporal = 50; // Asignar un índice
    animales_actual[0].agua = 1; // Asignar agua
    animales_actual[0].cantDosis = 3; // Asignar cantidad de dosis
    animales_actual[0].intervaloMin = 60; // Asignar intervalo    
}


// === ANIMALES LEÍDOS ===
data_animal_leido animales_leidos_actual[100] = {0};
data_animal_leido animales_leidos_copia[100] = {0};
SemaphoreHandle_t mutex_animales_leidos = NULL;

// === CURVAS ===
tipo_curva curvas_actual[5] = {0};
tipo_curva curvas_copia[5] = {0};
SemaphoreHandle_t mutex_curvas = NULL;



// === TIMESTAMPS ===
time_t timestamp_animales = 1752138999;
SemaphoreHandle_t mutex_Tanimales = NULL;
time_t copia_Tanimales = 0;

time_t timestamp_animales_leidos = 0;
SemaphoreHandle_t mutex_Tanimales_leidos = NULL;
time_t copia_Tanimales_leidos = 0;

time_t timestamp_curvas = 0;
SemaphoreHandle_t mutex_Tcurvas = NULL;
time_t copia_Tcurvas = 0;

time_t timestamp_configuracion = 0;
SemaphoreHandle_t mutex_Tconfiguracion = NULL;
time_t copia_Tconfiguracion = 0;

time_t timestamp_relojRTC = 0;

// === CONFIG TOLVA ===
ConfigTolva config_actual = {0};
SemaphoreHandle_t mutex_config = NULL;