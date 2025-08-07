// 📁 curva.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "cJSON.h"
#include "structyvar.h"

#define TAG "UART_RECEIVER_CURVA"
// Función auxiliar (opcional) para inicializar mutex si es necesario
tipo_curva curvas_aux = {0};
SemaphoreHandle_t get_mutex_curvas() {
    if (mutex_curvas == NULL) {
        mutex_curvas = xSemaphoreCreateMutex();
        if (mutex_curvas == NULL) {
            ESP_LOGE(TAG, "Error al crear mutex_curvas");
        }
    }
    return mutex_curvas;
}

void procesar_json_curva(const char *json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "Error al parsear JSON");
        return;
    }

    if (!cJSON_IsArray(root) || cJSON_GetArraySize(root) == 0) {
        ESP_LOGE(TAG, "JSON no es array con al menos 1 elemento");
        cJSON_Delete(root);
        return;
    }

    cJSON *curva = cJSON_GetArrayItem(root, 0);
    cJSON *segmentos = cJSON_GetObjectItem(curva, "segmentos");
    if (!cJSON_IsArray(segmentos)) {
        ESP_LOGE(TAG, "No se encontró 'segmentos'");
        cJSON_Delete(root);
        return;
    }

    if (get_mutex_curvas() && xSemaphoreTake(mutex_curvas, pdMS_TO_TICKS(100)) == pdTRUE) {
        int cantidad = cJSON_GetArraySize(segmentos);
        if (cantidad > 17) cantidad = 17;

        for (int i = 0; i < 17; i++) {
            if (i < cantidad) {
                cJSON *seg = cJSON_GetArrayItem(segmentos, i);
                cJSON *dia_item = cJSON_GetObjectItem(seg, "dia");
                cJSON *indice_item = cJSON_GetObjectItem(seg, "indice");

                if (cJSON_IsNumber(dia_item) && cJSON_IsString(indice_item)) {
                    curvas_aux.segmentos[i].inicio = dia_item->valueint;

                    // Convertir string "85%" a entero 85
                    char tmp[8];
                    strncpy(tmp, indice_item->valuestring, sizeof(tmp) - 1);
                    tmp[sizeof(tmp) - 1] = '\0';
                    char *p = strchr(tmp, '%');
                    if (p) *p = '\0';
                    curvas_aux.segmentos[i].pesoInicio = atoi(tmp);
                } else {
                    curvas_aux.segmentos[i].inicio = 0;
                    curvas_aux.segmentos[i].pesoInicio = 0;
                }
            } else {
                curvas_aux.segmentos[i].inicio = 0;
                curvas_aux.segmentos[i].pesoInicio = 0;
            }
        }

        xSemaphoreGive(mutex_curvas);
        ESP_LOGI(TAG, "Curva procesada correctamente con %d segmentos", cantidad);
    } else {
        ESP_LOGW(TAG, "No se pudo obtener el mutex de curvas");
    }
    // Actualizar curvas_actual con los datos procesados
    for(uint8_t i = 0; i < 5; i++) {
        if(curvas_actual[i].segmentos[0].inicio == 0){
            curvas_actual[i] = curvas_aux;
            break;
        }
    }
    cJSON_Delete(root);
}