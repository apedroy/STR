#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 3
#define NUM_CONSUMERS 2
#define NUM_ITEMS 20

typedef struct {
    int *buffer;
    int in;
    int out;
    sem_t mutex;
    sem_t empty;
    sem_t full;
} SharedBuffer;

typedef struct {
    SharedBuffer *buffer;
    int id;
} ProducerArgs;

typedef struct {
    SharedBuffer *buffer;
    int id;
} ConsumerArgs;

int item = 1; // Variável item definida como global
int totalProducedItems = 0; // Contador de itens produzidos global

void initBuffer(SharedBuffer *buffer) {
    buffer->buffer = (int *)malloc(BUFFER_SIZE * sizeof(int));
    buffer->in = 0;
    buffer->out = 0;
    sem_init(&buffer->mutex, 0, 1);
    sem_init(&buffer->empty, 0, BUFFER_SIZE);
    sem_init(&buffer->full, 0, 0);
}

void destroyBuffer(SharedBuffer *buffer) {
    sem_destroy(&buffer->mutex);
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);
    free(buffer->buffer);
}

void printBufferStatus(SharedBuffer *buffer) {
    printf("Items in buffer:");
    if (buffer->in > buffer->out) {
        for (int i = buffer->out; i < buffer->in; i++) {
            printf(" %d", buffer->buffer[i]);
        }
    } else if (buffer->in < buffer->out) {
        for (int i = buffer->out; i < BUFFER_SIZE; i++) {
            printf(" %d", buffer->buffer[i]);
        }
        for (int i = 0; i < buffer->in; i++) {
            printf(" %d", buffer->buffer[i]);
        }
    }
    printf("\n");
}

void produce(SharedBuffer *buffer, int id) {
    while (1) {
        sem_wait(&buffer->empty);
        sem_wait(&buffer->mutex);

        if (totalProducedItems >= NUM_ITEMS) {
            sem_post(&buffer->mutex);
            sem_post(&buffer->empty);
            break; // Sai do loop se o limite de produção total for atingido
        }

        buffer->buffer[buffer->in] = item;
        buffer->in = (buffer->in + 1) % BUFFER_SIZE;
        printf("Produtor %d: Produziu item %d\n", id, item++);
        totalProducedItems++;

        printBufferStatus(buffer); // Mostra os itens no buffer

        sem_post(&buffer->mutex);
        sem_post(&buffer->full);

        usleep(rand() % 100000);
    }
}

void consume(SharedBuffer *buffer, int id) {
    while (1) {
        sem_wait(&buffer->full);
        sem_wait(&buffer->mutex);

        int consumedItem = buffer->buffer[buffer->out];
        buffer->out = (buffer->out + 1) % BUFFER_SIZE;
        printf("Consumidor %d: Consumiu item %d\n", id, consumedItem);

        printBufferStatus(buffer); // Mostra os itens no buffer

        sem_post(&buffer->mutex);
        sem_post(&buffer->empty);

        usleep(rand() % 150000);
    }
}


void *producerThread(void *args) {
    ProducerArgs *pArgs = (ProducerArgs *)args;
    produce(pArgs->buffer, pArgs->id);
    pthread_exit(NULL);
}

void *consumerThread(void *args) {
    ConsumerArgs *cArgs = (ConsumerArgs *)args;
    consume(cArgs->buffer, cArgs->id);
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    
    SharedBuffer buffer;
    initBuffer(&buffer);

    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    ProducerArgs pArgs[NUM_PRODUCERS];
    ConsumerArgs cArgs[NUM_CONSUMERS];

    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pArgs[i].buffer = &buffer;
        pArgs[i].id = i + 1;
        pthread_create(&producers[i], NULL, producerThread, &pArgs[i]);
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        cArgs[i].buffer = &buffer;
        cArgs[i].id = i + 1;
        pthread_create(&consumers[i], NULL, consumerThread, &cArgs[i]);
    }

    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    destroyBuffer(&buffer);

    return 0;
}
