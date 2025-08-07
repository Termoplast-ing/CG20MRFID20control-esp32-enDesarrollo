#include "envio_animal_leido.h"
#include "structyvar.h"
#include "driver/uart.h"
#include "cJSON.h"
#include <string.h>
#include <stdio.h>

#define UART_ENVIO UART_NUM_1 // Puerto UART usado
#define TAG_DELIM_INICIO "<<<"
#define TAG_DELIM_FIN ">>>"

void enviar_animales_leidos_uart() {
    if (!mutex_animales_leidos) {
        printf("UART_ENVIO: mutex_animales_leidos no creado aún.\n");
        return;
    }

    // Paso 1: Tomar mutex
    if (!xSemaphoreTake(mutex_animales_leidos, pdMS_TO_TICKS(200))) {
        printf("UART_ENVIO: No se pudo tomar mutex_animales_leidos\n");
        return;
    }else{
        printf("UART_ENVIO: mutex_animales_leidos tomado\n");
        for(uint8_t i = 0; i < 100; i++) {
            animales_leidos_actual[i]=animales_leidos_copia[i];
        }
        xSemaphoreGive(mutex_animales_leidos);
    }
   

    // Paso 3: Limpiar animales_leidos_actual para liberar espacio
    memset(animales_leidos_copia, 0, sizeof(animales_leidos_copia));
        for(uint8_t i=0; i<100; i++){
    snprintf(animales_leidos_copia[i].nombre, sizeof(animales_leidos_copia[i].nombre), "000000000000000");
    }

    //imprimer todo el arreglo animales_leidos_copia pero solo si tiene datos
    printf("UART_ENVIO: Imprimiendo animales_leidos_actual:\n");
    for (int i = 0; i < 100; i++) {
        // Verificar si el animal_leidos_copia.nombre no es vacío
        if (strncmp(animales_leidos_actual[i].nombre, "000000000000000",15) != 0) {
            printf("Animal %d: %s\n", i, animales_leidos_actual[i].nombre);
            printf("Fecha dispensado: %lld\n", animales_leidos_actual[i].fechaDispensado);
            printf("Peso dispensado: %02x\n", animales_leidos_actual[i].pesoDispensado);
            printf("Nro Tolva: %02x\n", animales_leidos_actual[i].nroTolva);
        }
    }


    // Paso 5: Procesar y enviar datos usando animales_leidos_copia
    cJSON *json_array = cJSON_CreateArray();

    for (int i = 0; i < 100; i++) {
        if (strncmp(animales_leidos_actual[i].nombre, "000000000000000",15) != 0) {
            cJSON *animal = cJSON_CreateObject();
            cJSON_AddNumberToObject(animal, "corral", animales_leidos_actual[i].nroTolva);
            cJSON_AddStringToObject(animal, "caravana", animales_leidos_actual[i].nombre);
            cJSON_AddNumberToObject(animal, "fecha", animales_leidos_actual[i].fechaDispensado);
            cJSON_AddNumberToObject(animal, "peso", animales_leidos_actual[i].pesoDispensado);
            cJSON_AddItemToArray(json_array, animal);
        }
    }

    char *json_str = cJSON_PrintUnformatted(json_array);
    cJSON_Delete(json_array);

    if (json_str) {
        char buffer_final[1500];
        snprintf(buffer_final, sizeof(buffer_final), "%s%s%s", TAG_DELIM_INICIO, json_str, TAG_DELIM_FIN);
        uart_write_bytes(UART_ENVIO, buffer_final, strlen(buffer_final));
        printf("UART_ENVIO: Enviado JSON por UART:\n%s\n", buffer_final);
        free(json_str);
    } else {
        printf("UART_ENVIO: Fallo al crear el JSON\n");
    }
  memset(animales_leidos_actual, 0, sizeof(animales_leidos_actual));
  for(uint8_t i=0; i<100; i++){
    snprintf(animales_leidos_actual[i].nombre, sizeof(animales_leidos_actual[i].nombre), "000000000000000");
  }
}