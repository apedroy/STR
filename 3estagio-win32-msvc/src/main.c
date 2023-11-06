#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // Inclua esta biblioteca para usar o tipo bool
#include <time.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define ATTACK_TASK_PRIORITY 2
#define DEFENSE_TASK_PRIORITY 3

TaskHandle_t xAttackTaskHandle;
SemaphoreHandle_t xMissileSemaphore;
bool inhabitedAreas[10]; // Vetor que representa áreas habitadas

void attackTask(void *pvParameters) {
    srand(time(NULL));

    while (1) {
       printf("# iniciando novo ataque\n");
        // Gera ataques em intervalos aleatórios (exemplo: a cada 2 segundos)
        int interval = (rand() % 3 + 1) * 2000;
        vTaskDelay(interval / portTICK_PERIOD_MS);

        // Determina a quantidade aleatória de mísseis a serem disparados
        int missileCount = rand() % 7 + 1;

        for (int i = 0; i < missileCount; i++) {
            int target = rand() % 10;
            printf("Ataque: Missil #%d direcionado ao alvo %d\n", i + 1, target);

            if (inhabitedAreas[target]) {
                xSemaphoreGive(xMissileSemaphore); // Informa à defesa sobre o lançamento
            }
        }
    }
}

void defenseTask(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(xMissileSemaphore, portMAX_DELAY) == pdPASS) {
            // Defesa: Intercepta mísseis direcionados a áreas habitadas (neste exemplo, apenas exibe as informações)
            printf("Defesa: Missil inimigo interceptado.\n");
        }
    }
}

int main() {
    xMissileSemaphore = xSemaphoreCreateBinary();

    if (xMissileSemaphore == NULL) {
        printf("Erro ao criar semáforo.\n");
        return 1;
    }

    // Inicializa o vetor de áreas habitadas (por exemplo, áreas 2, 4 e 8 estão habitadas)
    for (int i = 0; i < 10; i++) {
        inhabitedAreas[i] = (i == 2 || i == 4 || i == 8);
    }

    // Inicialize o FreeRTOS

    // Crie a tarefa de ataque
    xTaskCreate(attackTask, "AttackTask", configMINIMAL_STACK_SIZE, NULL, ATTACK_TASK_PRIORITY, &xAttackTaskHandle);

    // Crie a tarefa de defesa
    xTaskCreate(defenseTask, "DefenseTask", configMINIMAL_STACK_SIZE, NULL, DEFENSE_TASK_PRIORITY, NULL);

    // Inicie o escalonador do FreeRTOS
    vTaskStartScheduler();

    return 0;
}