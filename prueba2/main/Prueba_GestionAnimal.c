/*#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "cJSON.h"

#define UART_PORT UART_NUM_1
#define BUF_SIZE 1024
#define TAG "UART_RECEIVER_ANIMAL"

static char buffer_animal[BUF_SIZE * 4];
static int pos_animal = 0;
static bool capturando_animal = false;

static int curva_str_a_int(const char *s) {
    if (strcmp(s, "Flaca") == 0) return 1;
    if (strcmp(s, "Mediana") == 0) return 2;
    if (strcmp(s, "Gorda") == 0) return 3;
    return -1;
}

static int indice_str_a_int(const char *s) {
    if (strstr(s, "Bajo") == s) return 1;
    if (strstr(s, "Normal") == s) return 2;
    if (strstr(s, "Alto") == s) return 3;
    return -1;
}

void procesar_json_animal(const char *json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (root && cJSON_IsArray(root)) {
        int n = cJSON_GetArraySize(root);
        for (int k = 0; k < n; k++) {
            cJSON *an = cJSON_GetArrayItem(root, k);
            const char *car = cJSON_GetObjectItem(an, "caravana")->valuestring;

            int64_t inse = 0;
            cJSON *ins = cJSON_GetObjectItem(an, "inseminacion");
            if (cJSON_IsNumber(ins)) inse = ins->valuedouble;
            else if (cJSON_IsString(ins)) inse = atoll(ins->valuestring);

            int agua = cJSON_GetObjectItem(an, "agua")->valueint;
            const char *cur_str = cJSON_GetObjectItem(an, "curva")->valuestring;
            int curva = curva_str_a_int(cur_str);
            const char *ind_str = cJSON_GetObjectItem(an, "indice")->valuestring;
            int indice = indice_str_a_int(ind_str);

            cJSON *ps = cJSON_GetObjectItem(an, "peso");
            int peso = cJSON_IsNumber(ps) ? (int)ps->valuedouble : atoi(ps->valuestring);

            int dosis = cJSON_GetObjectItem(an, "dosis")->valueint;
            int intervalo = cJSON_GetObjectItem(an, "intervalo")->valueint;

            ESP_LOGI(TAG, "animal: caravana=%s, inseminacion=%lld, agua=%d, curva=%d, indice=%d, peso=%d, dosis=%d, intervalo=%d",
                     car, inse, agua, curva, indice, peso, dosis, intervalo);
        }
        cJSON_Delete(root);
    } else {
        ESP_LOGE(TAG, "JSON inválido (animal): %s", json_str);
    }
}

static void uart_event_task_animal(void *arg) {
    QueueHandle_t uart_queue = (QueueHandle_t)arg;
    uart_event_t ev;
    uint8_t data[BUF_SIZE];

    while (true) {
        if (xQueueReceive(uart_queue, &ev, portMAX_DELAY) && ev.type == UART_DATA) {
            int len = uart_read_bytes(UART_PORT, data, ev.size, portMAX_DELAY);
            for (int i = 0; i < len; i++) {
                if (!capturando_animal && data[i]=='<' && i+2<len && data[i+1]=='<' && data[i+2]=='<') {
                    capturando_animal = true; pos_animal = 0; i += 2;
                    continue;
                }
                if (capturando_animal && data[i]=='>' && i+2<len && data[i+1]=='>' && data[i+2]=='>') {
                    buffer_animal[pos_animal] = '\0';
                    capturando_animal = false;
                    ESP_LOGI(TAG, "JSON recibido (animal): %s", buffer_animal);

                    procesar_json_animal(buffer_animal);

                    uart_write_bytes(UART_PORT, "Animal recibido\n", strlen("Animal recibido\n"));
                    i += 2;
                    continue;
                }
                if (capturando_animal && pos_animal < sizeof(buffer_animal)-1) {
                    buffer_animal[pos_animal++] = data[i];
                }
            }
        }
    }
}

void gestion_animal(QueueHandle_t q) {
    xTaskCreate(uart_event_task_animal, "animal_task", 8192, q, 10, NULL);
}*/

// 📁 config.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "cJSON.h"
#include "structyvar.h"
#include "reloj.h"

#define UART_PORT UART_NUM_1
#define BUF_SIZE 1024
#define TAG "UART_RECEIVER_ANIMAL"

//static char buffer_animal[BUF_SIZE * 4];
//static int pos_animal = 0;
//static bool capturando_animal = false;

static int curva_str_a_int(const char *s) {
    if (strcmp(s, "Ascendente") == 0) return 1;
    if (strcmp(s, "Constante") == 0) return 2;
    if (strcmp(s, "Descendente") == 0) return 3;
    if (strcmp(s, "Forma V") == 0) return 4;
    return 2;
}

static int indice_str_a_int(const char *s) {
    if (strstr(s, "Gorda") == s) return 1;
    if (strstr(s, "Normal") == s) return 2;
    if (strstr(s, "Flaca") == s) return 3;
    return 2;
}

void procesar_json_animal(const char *json_str) {
    read_time();

    timestamp_animales = RTC_time; // Actualizar timestamp al procesar nuevos datos
    cJSON *root = cJSON_Parse(json_str);
    if (!root || !cJSON_IsArray(root)) {
        ESP_LOGE(TAG, "JSON inválido (animal): %s", json_str);
        return;
    }

    int n = cJSON_GetArraySize(root);
    for (int k = 0; k < n; k++) {
        cJSON *an = cJSON_GetArrayItem(root, k);
        if (!an || !cJSON_IsObject(an)) continue;

        const char *car = cJSON_GetObjectItem(an, "caravana")->valuestring;

         if(car==NULL){
            car="";
        }else{
            if(strlen(car)>15){
                car="";
            }
        }

        time_t inse = 0;
        cJSON *ins = cJSON_GetObjectItem(an, "inseminacion");
        if(ins!=NULL){
            if (cJSON_IsNumber(ins)) inse = ins->valuedouble;
            else if (cJSON_IsString(ins)) inse = atoll(ins->valuestring);
        }else{
            inse=946695600; // 01/01/2000
        }

        uint8_t agua = cJSON_GetObjectItem(an, "agua")->valueint;

        const char *cur_str = cJSON_GetObjectItem(an, "curva")->valuestring;
        if(cur_str==NULL){
            cur_str="Constante";
        }
    
        uint8_t curva = curva_str_a_int(cur_str);

        const char *ind_str = cJSON_GetObjectItem(an, "indice")->valuestring;
        if(ind_str==NULL){
            ind_str="Normal";
        }
        uint8_t indice = indice_str_a_int(ind_str);

        cJSON *ps = cJSON_GetObjectItem(an, "peso");
        uint8_t peso = 0;
        if(ps!=NULL){
            if(cJSON_IsNumber(ps)){
                peso = (int)ps->valuedouble;
            }else if(cJSON_IsString(ps)){
                peso = atoi(ps->valuestring);
            }
        }else{
            peso=0;
        }
        //uint8_t peso = cJSON_IsNumber(ps) ? (int)ps->valuedouble : atoi(ps->valuestring);

        uint8_t dosis = cJSON_GetObjectItem(an, "dosis")->valueint;
        uint8_t intervalo = cJSON_GetObjectItem(an, "intervalo")->valueint;

        ESP_LOGI(TAG, "Animal: caravana=%s, inseminacion=%lld, agua=%d, curva=%d, indice=%d, peso=%d, dosis=%d, intervalo=%d",
                 car, inse, agua, curva, indice, peso, dosis, intervalo);
        int aux=-1;
        for(uint8_t j=0; j<20 ; j++){
            printf("Comparando con animal índice %d: %s\n", j, animales_actual[j].nombre);
            printf("car: %s\n", car);
            if(strcmp(animales_actual[j].nombre, car)==0){
                aux=j;
                break;
            }
        }
        printf("aux después de buscar coincidencia: %d\n", aux);
        if(aux==-1){
            for(uint8_t j=0; j<20 ; j++){
                printf("Comparando con animal índice %d: %s\n", j, animales_actual[j].nombre);
            printf("car: %s\n", car);
                if((strcmp(animales_actual[j].nombre, "000000000000000")==0)||((animales_actual[j].nombre[0]=='\0'))){
                    aux=j;
                    break;
                }
            }
        }
        if(aux>=0 && aux<20){
            //printf("aux: %d\n", aux);
            //printf("car: %s\n", car);
            //
        printf("Asignando datos al animal índice %d\n", aux);
                 for(uint8_t i =0 ; i<16; i++){
            if (i < strlen(car)) {
                animales_actual[aux].nombre[i] = car[i];
                //printf("%c", animales_actual[k].nombre[i]);
            } //else {
              //animales_actual[indice].nombre[k] = '0'; // Rellenar con '0' si es necesario
                
            //}
        }
        timestamp_animales=time(NULL);
        animales_actual[aux].nombre[15] = '\0'; // Asegurar el fin de cadena
        animales_actual[aux].tipoCurva = curva; // Asignar tipo de curva
        animales_actual[aux].pesoDosis = peso; // Asignar peso
        animales_actual[aux].fechaServicio = inse; // Asignar fecha de servicio
        animales_actual[aux].agua = agua; // Asignar agua
        animales_actual[aux].indiceCorporal = indice; // Asignar índice corporal
        animales_actual[aux].cantDosis = dosis; // Asignar peso
        animales_actual[aux].intervaloMin = intervalo; // Asignar intervalo
    }else{
    ESP_LOGW(TAG, "No hay espacio para más animales o índice inválido");
    }/* printf("%d\n", k);
        printf("%d\n", peso);
        printf("%d\n", animales_actual[k].pesoDosis);
        printf("%d\n", animales_actual[k].tipoCurva);
        printf("%d\n", animales_actual[k].agua);
        printf("%lld\n", animales_actual[k].fechaServicio);
        printf("%d\n", animales_actual[k].indiceCorporal);
        }*/
    }
    cJSON_Delete(root);
}