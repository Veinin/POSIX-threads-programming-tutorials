#include <pthread.h>
#include <semaphore.h>
#include "errors.h"

#define BUFF_SIZE 10
#define PRODUCTER_SIZE 5
#define CONSUMER_SIZE 5
#define ITERATIONS 10

typedef struct buff_type {
    int buff[BUFF_SIZE];
    int in;
    int out;
    sem_t full;
    sem_t empty;
    sem_t mutex;
} buff_t;

buff_t shared;

void *producter(void *arg)
{
    int id = *(int*)arg;
    int item;
    int count;

    for (count = 0; count < ITERATIONS; count++) {
        sem_wait(&shared.empty);
        sem_wait(&shared.mutex);

        item = count;
        shared.buff[shared.in] = item;
        shared.in++;
        shared.in %= BUFF_SIZE;

        printf("[%d] Producing %d ...\n", id, item);
        fflush(stdout);

        sem_post(&shared.mutex);
        sem_post(&shared.full);

        if (count % 2 == 1)
            sleep(1);
    }
}

void *consumer(void *arg)
{
    int id = *(int *)arg;
    int item;
    int count;

    for (count = 0; count < ITERATIONS; count++) {
        sem_wait(&shared.full);
        sem_wait(&shared.mutex);

        item = shared.buff[shared.out];
        shared.out++;
        shared.out %= BUFF_SIZE;

        printf("[%d] Consuming %d ...\n", id, item);
        fflush(stdout);

        sem_post(&shared.mutex);
        sem_post(&shared.empty);

        if (count % 2 == 1)
            sleep(1);
    }
}

int main()
{
    pthread_t producter_id, consumer_id;
    int count;
    int status;

    status = sem_init(&shared.full, 0, 0);
    if (status != 0)
        err_abort(status, "Sem init");

    status = sem_init(&shared.empty, 0, BUFF_SIZE);
    if (status != 0)
        err_abort(status, "Sem init");

    status = sem_init(&shared.mutex, 0, BUFF_SIZE);
    if (status != 0)
        err_abort(status, "Sem init");

    for (count = 0; count < PRODUCTER_SIZE; count++){
        pthread_create(&producter_id, NULL, producter, (void *)&count);
    }

    for (count = 0; count < CONSUMER_SIZE; count++) {
        pthread_create(&consumer_id, NULL, consumer, (void *)&count);
    }

    pthread_exit(NULL);
}