#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h" 
#include "structyvar.h"
#include "time.h"

// === CONFIGURACIÓN ===
configuration configuracion_actual = {0};
configuration configuracion_copia = {0};
SemaphoreHandle_t mutex_configuracion = NULL;
//uint8_t response[9] = {0}; // Respuesta del Modbus
uint8_t indice = 0; // Índice para animales y curvas
// === ANIMALES ===
data_animal animales_actual[20] = {0};
data_animal animales_copia[20] = {0};
SemaphoreHandle_t mutex_animales = NULL;
bool timeOK = false; // Variable para indicar si el tiempo está sincronizado


// Inicialización de nombre para animales_actual[0]
void inicializar_animales_actual_nombre() {
	animales_actual[0].nombre[0] = '9';
	animales_actual[0].nombre[1] = '9';
	animales_actual[0].nombre[2] = '9';
	animales_actual[0].nombre[3] = '0';
	animales_actual[0].nombre[4] = '0';
	animales_actual[0].nombre[5] = '0';
	animales_actual[0].nombre[6] = '2';
	animales_actual[0].nombre[7] = '0';
	animales_actual[0].nombre[8] = '2';
	animales_actual[0].nombre[9] = '4';
	animales_actual[0].nombre[10] = '0';
	animales_actual[0].nombre[11] = '0';
	animales_actual[0].nombre[12] = '0';
	animales_actual[0].nombre[13] = '0';
	animales_actual[0].nombre[14] = '3';
	animales_actual[0].nombre[15] = '\0'; // Asegurar el fin de cadena
    animales_actual[0].tipoCurva = 1; // Asignar un tipo de curva
    animales_actual[0].pesoDosis = 10; // Asignar un peso
    animales_actual[0].fechaServicio = 1752807600; // Asignar fecha de servicio
    animales_actual[0].indiceCorporal = 50; // Asignar un índice
    animales_actual[0].agua = 1; // Asignar agua
    animales_actual[0].cantDosis = 3; // Asignar cantidad de dosis
    animales_actual[0].intervaloMin = 60; // Asignar intervalo    
	//animal 2
	animales_actual[1].nombre[0] = '9';
	animales_actual[1].nombre[1] = '9';
	animales_actual[1].nombre[2] = '9';
	animales_actual[1].nombre[3] = '0';
	animales_actual[1].nombre[4] = '0';
	animales_actual[1].nombre[5] = '0';
	animales_actual[1].nombre[6] = '2';
	animales_actual[1].nombre[7] = '0';
	animales_actual[1].nombre[8] = '2';
	animales_actual[1].nombre[9] = '4';
	animales_actual[1].nombre[10] = '0';
	animales_actual[1].nombre[11] = '0';
	animales_actual[1].nombre[12] = '0';
	animales_actual[1].nombre[13] = '0';
	animales_actual[1].nombre[14] = '4';
	animales_actual[1].nombre[15] = '\0'; // Asegurar el fin de cadena
    animales_actual[1].tipoCurva = 2; // Asignar un tipo de curva
    animales_actual[1].pesoDosis = 20; // Asignar un peso
    animales_actual[1].fechaServicio = 1752807600; // Asignar fecha de servicio
    animales_actual[1].indiceCorporal = 50; // Asignar un índice
    animales_actual[1].agua = 0; // Asignar agua
    animales_actual[1].cantDosis = 3; // Asignar cantidad de dosis
    animales_actual[1].intervaloMin = 60; // Asignar intervalo    
	//animal 3
	animales_actual[2].nombre[0] = '9';
	animales_actual[2].nombre[1] = '9';
	animales_actual[2].nombre[2] = '9';
	animales_actual[2].nombre[3] = '0';
	animales_actual[2].nombre[4] = '0';
	animales_actual[2].nombre[5] = '0';
	animales_actual[2].nombre[6] = '2';
	animales_actual[2].nombre[7] = '0';
	animales_actual[2].nombre[8] = '2';
	animales_actual[2].nombre[9] = '4';
	animales_actual[2].nombre[10] = '0';
	animales_actual[2].nombre[11] = '0';
	animales_actual[2].nombre[12] = '0';
	animales_actual[2].nombre[13] = '0';
	animales_actual[2].nombre[14] = '6';
	animales_actual[2].nombre[15] = '\0'; // Asegurar el fin de cadena
    animales_actual[2].tipoCurva = 3; // Asignar un tipo de curva
    animales_actual[2].pesoDosis = 35; // Asignar un peso
    animales_actual[2].fechaServicio = 1752807600; // Asignar fecha de servicio
    animales_actual[2].indiceCorporal = 75; // Asignar un índice
    animales_actual[2].agua = 1; // Asignar agua
    animales_actual[2].cantDosis = 1; // Asignar cantidad de dosis
    animales_actual[2].intervaloMin = 120; // Asignar intervalo    
	//animal 4
	animales_actual[3].nombre[0] = '9';
	animales_actual[3].nombre[1] = '9';
	animales_actual[3].nombre[2] = '9';
	animales_actual[3].nombre[3] = '0';
	animales_actual[3].nombre[4] = '0';
	animales_actual[3].nombre[5] = '0';
	animales_actual[3].nombre[6] = '2';
	animales_actual[3].nombre[7] = '0';
	animales_actual[3].nombre[8] = '2';
	animales_actual[3].nombre[9] = '4';
	animales_actual[3].nombre[10] = '0';
	animales_actual[3].nombre[11] = '0';
	animales_actual[3].nombre[12] = '0';
	animales_actual[3].nombre[13] = '1';
	animales_actual[3].nombre[14] = '8';
	animales_actual[3].nombre[15] = '\0'; // Asegurar el fin de cadena
    animales_actual[3].tipoCurva = 1; // Asignar un tipo de curva
    animales_actual[3].pesoDosis = 15; // Asignar un peso
    animales_actual[3].fechaServicio = 1752116400; // Asignar fecha de servicio
    animales_actual[3].indiceCorporal = 100; // Asignar un índice
    animales_actual[3].agua = 1; // Asignar agua
    animales_actual[3].cantDosis = 1; // Asignar cantidad de dosis
    animales_actual[3].intervaloMin = 00; // Asignar intervalo    
	//animal 5
	animales_actual[4].nombre[0] = '9';
	animales_actual[4].nombre[1] = '9';
	animales_actual[4].nombre[2] = '9';
	animales_actual[4].nombre[3] = '0';
	animales_actual[4].nombre[4] = '0';
	animales_actual[4].nombre[5] = '0';
	animales_actual[4].nombre[6] = '2';
	animales_actual[4].nombre[7] = '0';
	animales_actual[4].nombre[8] = '2';
	animales_actual[4].nombre[9] = '4';
	animales_actual[4].nombre[10] = '0';
	animales_actual[4].nombre[11] = '0';
	animales_actual[4].nombre[12] = '0';
	animales_actual[4].nombre[13] = '1';
	animales_actual[4].nombre[14] = '9';
	animales_actual[4].nombre[15] = '\0'; // Asegurar el fin de cadena
    animales_actual[4].tipoCurva = 2; // Asignar un tipo de curva
    animales_actual[4].pesoDosis = 40; // Asignar un peso
    animales_actual[4].fechaServicio = 1752116400; // Asignar fecha de servicio
    animales_actual[4].indiceCorporal = 150; // Asignar un índice
    animales_actual[4].agua = 0; // Asignar agua
    animales_actual[4].cantDosis = 3; // Asignar cantidad de dosis
    animales_actual[4].intervaloMin = 180; // Asignar intervalo    
	//animal 6
	animales_actual[5].nombre[0] = '9';
	animales_actual[5].nombre[1] = '9';
	animales_actual[5].nombre[2] = '9';
	animales_actual[5].nombre[3] = '0';
	animales_actual[5].nombre[4] = '0';
	animales_actual[5].nombre[5] = '0';
	animales_actual[5].nombre[6] = '2';
	animales_actual[5].nombre[7] = '0';
	animales_actual[5].nombre[8] = '2';
	animales_actual[5].nombre[9] = '4';
	animales_actual[5].nombre[10] = '0';
	animales_actual[5].nombre[11] = '0';
	animales_actual[5].nombre[12] = '0';
	animales_actual[5].nombre[13] = '2';
	animales_actual[5].nombre[14] = '0';
	animales_actual[5].nombre[15] = '\0'; // Asegurar el fin de cadena
    animales_actual[5].tipoCurva = 3; // Asignar un tipo de curva
    animales_actual[5].pesoDosis = 50; // Asignar un peso
    animales_actual[5].fechaServicio = 1752116400; // Asignar fecha de servicio
    animales_actual[5].indiceCorporal = 200; // Asignar un índice
    animales_actual[5].agua = 1; // Asignar agua
    animales_actual[5].cantDosis = 3; // Asignar cantidad de dosis
    animales_actual[5].intervaloMin = 60; // Asignar intervalo 
	
	timestamp_animales = 1752138999;
}

void inicializar_config(){
	configuracion_actual.calibracionAgua=50;
	configuracion_actual.calibracionMotor=15;
	configuracion_actual.pesoAnimalDesconocido=10;
	configuracion_actual.caravanaLibre1[0] = '9';
	configuracion_actual.caravanaLibre1[1] = '9';
	configuracion_actual.caravanaLibre1[2] = '9';
	configuracion_actual.caravanaLibre1[3] = '0';
	configuracion_actual.caravanaLibre1[4] = '0';
	configuracion_actual.caravanaLibre1[5] = '0';
	configuracion_actual.caravanaLibre1[6] = '2';
	configuracion_actual.caravanaLibre1[7] = '0';
	configuracion_actual.caravanaLibre1[8] = '2';
	configuracion_actual.caravanaLibre1[9] = '4';
	configuracion_actual.caravanaLibre1[10] = '0';
	configuracion_actual.caravanaLibre1[11] = '0';
	configuracion_actual.caravanaLibre1[12] = '0';
	configuracion_actual.caravanaLibre1[13] = '0';
	configuracion_actual.caravanaLibre1[14] = '8';
	configuracion_actual.caravanaLibre1[15] = '\0'; // Asegurar el
	
	configuracion_actual.caravanaLibre2[0] = '9';
	configuracion_actual.caravanaLibre2[1] = '9';
	configuracion_actual.caravanaLibre2[2] = '9';
	configuracion_actual.caravanaLibre2[3] = '0';
	configuracion_actual.caravanaLibre2[4] = '0';
	configuracion_actual.caravanaLibre2[5] = '0';
	configuracion_actual.caravanaLibre2[6] = '2';
	configuracion_actual.caravanaLibre2[7] = '0';
	configuracion_actual.caravanaLibre2[8] = '2';
	configuracion_actual.caravanaLibre2[9] = '4';
	configuracion_actual.caravanaLibre2[10] = '0';
	configuracion_actual.caravanaLibre2[11] = '0';
	configuracion_actual.caravanaLibre2[12] = '0';
	configuracion_actual.caravanaLibre2[13] = '0';
	configuracion_actual.caravanaLibre2[14] = '9';
	configuracion_actual.caravanaLibre2[15] = '\0';

	configuracion_actual.caravanaLibre3[0] = '0';
	configuracion_actual.caravanaLibre3[1] = '0';
	configuracion_actual.caravanaLibre3[2] = '0';
	configuracion_actual.caravanaLibre3[3] = '0';
	configuracion_actual.caravanaLibre3[4] = '0';
	configuracion_actual.caravanaLibre3[5] = '0';
	configuracion_actual.caravanaLibre3[6] = '0';
	configuracion_actual.caravanaLibre3[7] = '0';
	configuracion_actual.caravanaLibre3[8] = '0';
	configuracion_actual.caravanaLibre3[9] = '0';
	configuracion_actual.caravanaLibre3[10] = '0';
	configuracion_actual.caravanaLibre3[11] = '0';
	configuracion_actual.caravanaLibre3[12] = '0';
	configuracion_actual.caravanaLibre3[13] = '0';
	configuracion_actual.caravanaLibre3[14] = '0';
	configuracion_actual.caravanaLibre3[15] = '\0';

	configuracion_actual.caravanaLibre4[0] = '0';
	configuracion_actual.caravanaLibre4[1] = '0';
	configuracion_actual.caravanaLibre4[2] = '0';
	configuracion_actual.caravanaLibre4[3] = '0';
	configuracion_actual.caravanaLibre4[4] = '0';
	configuracion_actual.caravanaLibre4[5] = '0';
	configuracion_actual.caravanaLibre4[6] = '0';
	configuracion_actual.caravanaLibre4[7] = '0';
	configuracion_actual.caravanaLibre4[8] = '0';
	configuracion_actual.caravanaLibre4[9] = '0';
	configuracion_actual.caravanaLibre4[10] = '0';
	configuracion_actual.caravanaLibre4[11] = '0';
	configuracion_actual.caravanaLibre4[12] = '0';
	configuracion_actual.caravanaLibre4[13] = '0';
	configuracion_actual.caravanaLibre4[14] = '0';
	configuracion_actual.caravanaLibre4[15] = '\0';

	configuracion_actual.caravanaLibre5[0] = '0';
	configuracion_actual.caravanaLibre5[1] = '0';
	configuracion_actual.caravanaLibre5[2] = '0';
	configuracion_actual.caravanaLibre5[3] = '0';
	configuracion_actual.caravanaLibre5[4] = '0';
	configuracion_actual.caravanaLibre5[5] = '0';
	configuracion_actual.caravanaLibre5[6] = '0';
	configuracion_actual.caravanaLibre5[7] = '0';
	configuracion_actual.caravanaLibre5[8] = '0';
	configuracion_actual.caravanaLibre5[9] = '0';
	configuracion_actual.caravanaLibre5[10] = '0';
	configuracion_actual.caravanaLibre5[11] = '0';
	configuracion_actual.caravanaLibre5[12] = '0';
	configuracion_actual.caravanaLibre5[13] = '0';
	configuracion_actual.caravanaLibre5[14] = '0';
	configuracion_actual.caravanaLibre5[15] = '\0';
}

void inicializar_curvas(){
	for(uint8_t i=0;i<5;i++){
		for(uint8_t j=0;j<18;j++){
			curvas_actual[i].segmentos[j].inicio = (i+j);
			curvas_actual[i].segmentos[j].pesoInicio = (i+j)+34;
		}
	}
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

time_t timestamp_curvas = 1752138999;
SemaphoreHandle_t mutex_Tcurvas = NULL;
time_t copia_Tcurvas = 0;

time_t timestamp_configuracion = 1752138999;
SemaphoreHandle_t mutex_Tconfiguracion = NULL;
time_t copia_Tconfiguracion = 0;

time_t timestamp_relojRTC = 0;

// === CONFIG TOLVA ===
ConfigTolva config_actual = {0};
SemaphoreHandle_t mutex_config = NULL;