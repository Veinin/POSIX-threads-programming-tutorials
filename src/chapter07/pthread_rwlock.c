#include <pthread.h>
#include "errors.h"

#define THREADS     5
#define DATASIZE    15
#define ITERATIONS  10000

typedef struct thread_tag {
    int         thread_num;
    pthread_t   thread_id;
    int         updates;
    int         reads;
    int         interval;
} thread_t;

typedef struct data_tag {
    pthread_rwlock_t    lock;
    int                 data;
    int                 updates;
} data_t;

thread_t threads[THREADS];
data_t  data[DATASIZE];

void *thread_routine(void *arg)
{
    thread_t *self = (thread_t*)arg;
    int repeats = 0;
    int iteration;
    int element = 0;
    int status;

    for (iteration = 0; iteration < ITERATIONS; iteration++) {
        if (iteration % self->interval == 0) {
            status = pthread_rwlock_wrlock(&data[element].lock);
            if (status != 0)
                err_abort(status, "Write lock");

            data[element].data = self->thread_num;
            data[element].updates++;
            self->updates++;

            status = pthread_rwlock_unlock(&data[element].lock);
            if (status != 0)
                err_abort(status, "Write unlock");
        } else {
            status = pthread_rwlock_rdlock(&data[element].lock);
            if (status != 0)
                err_abort(status, "Read lock");

            self->reads++;
            if (data[element].data == self->thread_num)
                repeats++;

            status = pthread_rwlock_unlock(&data[element].lock);
            if (status != 0)
                err_abort(status, "Read unlock");
        }

        element++;
        if (element >= DATASIZE)
            element = 0;
    }

    if (repeats > 0)
        printf("Thread %d found unchanged elements %d times\n",
            self->thread_num, repeats);
    return NULL;
}

int main()
{
    int count;
    int data_count;
    int status;
    unsigned int seed = 1;
    int thread_updates = 0;
    int data_updates = 0;

    for (data_count = 0; data_count < DATASIZE; data_count++) {
        data[data_count].data = 0;
        data[data_count].updates = 0;
        status = pthread_rwlock_init(&data[data_count].lock, NULL);
        if (status != 0)
            err_abort(status, "Init rw lock");
    }

    for (count = 0; count < THREADS; count++) {
        threads[count].thread_num = count;
        threads[count].updates = 0;
        threads[count].reads = 0;
        threads[count].interval = rand_r(&seed) % 7;
        status = pthread_create(&threads[count].thread_id, 
            NULL, thread_routine, (void *)&threads[count]);
        if (status != 0)
            err_abort(status, "Create thread");
    }

    for (count = 0; count < THREADS; count++) {
        status = pthread_join(threads[count].thread_id, NULL);
        if (status != 0)
            err_abort(status, "Join thread");

        thread_updates += threads[count].updates;
        printf("%02d: interval %d, updated %d, reads %d\n",
            count, threads[count].interval,
            threads[count].updates, threads[count].reads);
    }

    for (data_count = 0; data_count < DATASIZE; data_count++) {
        data_updates += data[data_count].updates;
        printf("data %02d: value %d, %d updates\n",
            data_count, data[data_count].data,
            data[data_count].updates);
        pthread_rwlock_destroy(&data[data_count].lock);
    }

    printf("%d thread updates, %d data updates\n", thread_updates, data_updates);
    return 0;
}