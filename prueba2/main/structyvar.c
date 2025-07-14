#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h" 
#include "structyvar.h"

// === CONFIGURACIÓN ===
configuration configuracion_actual = {0};
configuration configuracion_copia = {0};
SemaphoreHandle_t mutex_configuracion = NULL;

// === ANIMALES ===
data_animal animales_actual[20] = {0};
data_animal animales_copia[20] = {0};
SemaphoreHandle_t mutex_animales = NULL;

// === ANIMALES LEÍDOS ===
data_animal_leido animales_leidos_actual[100] = {0};
data_animal_leido animales_leidos_copia[100] = {0};
SemaphoreHandle_t mutex_animales_leidos = NULL;

// === CURVAS ===
tipo_curva curvas_actual[5] = {0};
tipo_curva curvas_copia[5] = {0};
SemaphoreHandle_t mutex_curvas = NULL;



// === TIMESTAMPS ===
time_t timestamp_animales = 0;
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