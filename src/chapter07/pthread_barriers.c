#include <pthread.h>
#include "errors.h"

#define THREADS 5
#define ARRAY 6
#define INLOOPS 1000
#define OUTLOOPS 10

typedef struct thread_tag {
    pthread_t   thread_id;
    int         number;
    int         increment;
    int         array[ARRAY];
} thread_t;

pthread_barrier_t barrier;
thread_t threads[THREADS];

void *thread_routine(void *arg)
{
    thread_t *self = (thread_t*)arg;
    int in_loop, out_loop, count, status;

    for (out_loop = 0; out_loop < OUTLOOPS; out_loop++) {
        status = pthread_barrier_wait(&barrier);
        if (status > 0)
            err_abort(status, "Wait on barrier");

        for (in_loop = 0; in_loop < INLOOPS; in_loop++)
            for (count = 0; count < ARRAY; count++)
                self->array[count] += self->increment;

        status = pthread_barrier_wait(&barrier);
        if (status > 0)
            err_abort(status, "Wait on barrier");

        if (status == -1) {
            int thread_num;

            for (thread_num = 0; thread_num < THREADS; thread_num++)
                threads[thread_num].increment += 1;
        }
    }

    return NULL;
}

int main()
{
    int thread_count, array_count;
    int status;
    
    pthread_barrier_init(&barrier, NULL, THREADS);

    for (thread_count = 0; thread_count < THREADS; thread_count++) {
        threads[thread_count].increment = thread_count;
        threads[thread_count].number = thread_count;

        for (array_count = 0; array_count < ARRAY; array_count++)
            threads[thread_count].array[array_count] = array_count + 1;

        status = pthread_create(&threads[thread_count].thread_id, NULL, thread_routine, (void*)&threads[thread_count]);
        if (status != 0)
            err_abort(status, "Create threads");
    }

    for (thread_count = 0; thread_count < THREADS; thread_count++) {
        status = pthread_join(threads[thread_count].thread_id, NULL);
        if (status != 0)
            err_abort(status, "Join threads");

        printf("%02d: (%d)", thread_count, threads[thread_count].increment);

        for (array_count = 0; array_count < ARRAY; array_count++)
            printf("%010u ", threads[thread_count].array[array_count]);

        printf("\n"); 
    }

    pthread_barrier_destroy(&barrier);
    return 0;
}