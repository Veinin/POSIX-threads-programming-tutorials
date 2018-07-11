#include <pthread.h>
#include "errors.h"

#define THREADS     10
#define ITERATIONS  100000

typedef struct thread_tag {
    int         thread_num;
    pthread_t   thread_id;
} thread_t;

thread_t threads[THREADS];
pthread_spinlock_t spl;
int updates = 0;

void *thread_routine(void *arg)
{
    thread_t *self = (thread_t*)arg;
    int iteration;

    for (iteration = 0; iteration < ITERATIONS; iteration++) {
        pthread_spin_lock(&spl);

        updates++;

        pthread_spin_unlock(&spl);
    }

    return NULL;
}

int main()
{
    int status;
    int count;
    int thread_updates = 0;
    unsigned int seed = 1;

    pthread_spin_init(&spl, PTHREAD_PROCESS_PRIVATE);

    for (count = 0; count < THREADS; count++) {
        threads[count].thread_num = count;
        status = pthread_create(&threads[count].thread_id, 
            NULL, thread_routine, (void *)&threads[count]);
        if (status != 0)
            err_abort(status, "Create thread");
    }

    for (count = 0; count < THREADS; count++) {
        status = pthread_join(threads[count].thread_id, NULL);
        if (status != 0)
            err_abort(status, "Join thread");
    }

    pthread_spin_destroy(&spl);

    printf("%d data updates\n", updates);

    return 0;
}