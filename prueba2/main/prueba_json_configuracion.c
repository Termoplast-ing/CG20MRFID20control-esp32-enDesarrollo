#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "structyvar.h"
#include "cJSON.h"
#include "reloj.h"

#define UART_PORT UART_NUM_1
#define BUF_SIZE 1024
#define TAG "UART_RECEIVER_CONFIG"

static char buffer_config[BUF_SIZE * 4];
static int pos_config = 0;
static bool capturando_config = false;
static int cfg_motor = 0;
static int cfg_agua = 0;
static int cfg_peso = 0;
#define MAX_CARAVANAS 5
static char *cfg_caravanas[MAX_CARAVANAS];
static int cfg_n_caravanas = 0;
static char cfg_tipo_indice[64] = {0};
static int cfg_porcentaje_indice = 0;

void procesar_json_config(const char *json_str) {
    read_time();

    timestamp_configuracion = RTC_time; // Actualizar timestamp al procesar nuevos datos
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "Error al parsear JSON");
        return;
    }

    cJSON *obj = NULL;
    if (cJSON_IsArray(root) && cJSON_GetArraySize(root) > 0)
        obj = cJSON_GetArrayItem(root, 0);
    else if (cJSON_IsObject(root))
        obj = root;

    if (!obj) {
        ESP_LOGE(TAG, "Formato JSON inesperado");
        cJSON_Delete(root);
        return;
    }

    cJSON *cal = cJSON_GetObjectItem(obj, "calibraciones");
    if (cJSON_IsObject(cal)) {
        cfg_motor = cJSON_GetObjectItem(cal, "motor")->valueint;
        cfg_agua = cJSON_GetObjectItem(cal, "agua")->valueint;
        cfg_peso = (int)(cJSON_GetObjectItem(cal, "peso")->valuedouble * 10.0 + 0.5);
        ESP_LOGI(TAG, "Motor=%d, Agua=%d, Peso=%d", cfg_motor, cfg_agua, cfg_peso);
        configuracion_actual.calibracionMotor = cfg_motor;
        configuracion_actual.calibracionAgua = cfg_agua;
        configuracion_actual.pesoAnimalDesconocido = cfg_peso;
    } else {
        ESP_LOGW(TAG, "Falta calibraciones");
    }

    cJSON *car = cJSON_GetObjectItem(obj, "caravanas_libres");
    if (cJSON_IsArray(car)) {
        cfg_n_caravanas = cJSON_GetArraySize(car);
        if (cfg_n_caravanas > MAX_CARAVANAS) cfg_n_caravanas = MAX_CARAVANAS;
    
        for (int i = 0; i < cfg_n_caravanas; i++) {
            cJSON *it = cJSON_GetArrayItem(car, i);
            const char *caravana_str;
    
            // Si el ítem es un string válido y no está vacío, lo usamos
            if (cJSON_IsString(it) && it->valuestring && strlen(it->valuestring) > 0) {
                caravana_str = it->valuestring;
            } else {
                // Si no hay dato válido, ponemos "0"
                caravana_str = "0";
            }
    
            // Guardar en las variables globales
            switch (i) {
                case 0:
                    strncpy((char *)configuracion_actual.caravanaLibre1, caravana_str, 15);
                    configuracion_actual.caravanaLibre1[15] = '\0';
                    break;
                case 1:
                    strncpy((char *)configuracion_actual.caravanaLibre2, caravana_str, 15);
                    configuracion_actual.caravanaLibre2[15] = '\0';
                    break;
                case 2:
                    strncpy((char *)configuracion_actual.caravanaLibre3, caravana_str, 15);
                    configuracion_actual.caravanaLibre3[15] = '\0';
                    break;
                case 3:
                    strncpy((char *)configuracion_actual.caravanaLibre4, caravana_str, 15);
                    configuracion_actual.caravanaLibre4[15] = '\0';
                    break;
                case 4:
                    strncpy((char *)configuracion_actual.caravanaLibre5, caravana_str, 15);
                    configuracion_actual.caravanaLibre5[15] = '\0';
                    break;
            }
    
            //ESP_LOGI(TAG, "Caravana[%d]=%s", i, caravana_str);
        }
    
        /*printf("Caravanas libres guardadas:\n");
        printf("1: %s\n", configuracion_actual.caravanaLibre1);
        printf("2: %s\n", configuracion_actual.caravanaLibre2);
        printf("3: %s\n", configuracion_actual.caravanaLibre3);
        printf("4: %s\n", configuracion_actual.caravanaLibre4);
        printf("5: %s\n", configuracion_actual.caravanaLibre5);*/
    } else {
        ESP_LOGW(TAG, "Falta caravana");
    }

    /*cJSON *ind = cJSON_GetObjectItem(obj, "indice_corporal");
    if (cJSON_IsObject(ind)) {
        strncpy(cfg_tipo_indice, cJSON_GetObjectItem(ind, "tipo")->valuestring, sizeof(cfg_tipo_indice)-1);
        char tmp[16];
        strncpy(tmp, cJSON_GetObjectItem(ind, "porcentaje")->valuestring, sizeof(tmp)-1);
        tmp[sizeof(tmp)-1] = '\0';
        char *p = strchr(tmp, '%');
        if (p) *p = '\0';
        cfg_porcentaje_indice = atoi(tmp);
        ESP_LOGI(TAG, "Indice corporal=%s (%d%%)", cfg_tipo_indice, cfg_porcentaje_indice);
    } else {
        ESP_LOGW(TAG, "Falta indice_corporal");
    }*/
    timestamp_configuracion=time(NULL);
    cJSON_Delete(root);
    ESP_LOGI(TAG, "Configuración procesada correctamente");
}