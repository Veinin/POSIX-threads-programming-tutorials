#include <pthread.h>
#include "errors.h"

#define THREADS 5

typedef struct control_tag
{
    int counter, bysy;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} control_t;

control_t control = {0, 1, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};

void cleanup_handler(void *arg)
{
    control_t *st = (control_t *)arg;
    int status;

    st->counter--;
    printf("cleaup_handler: counter == %d\n", st->counter);

    status = pthread_mutex_unlock(&st->mutex);
    if (status != 0)
        err_abort(status, "Unlock in cleanup handler");
}

void *thread_routine(void *arg)
{
    int status;

    pthread_cleanup_push(cleanup_handler, (void *)&control);

    status = pthread_mutex_lock(&control.mutex);
    if (status != 0)
        err_abort(status, "Mutex lock");

    control.counter++;

    while (control.bysy)
    {
        status = pthread_cond_wait(&control.cv, &control.mutex);
        if (status != 0)
            err_abort(status, "Wait on condition");
    }

    pthread_cleanup_pop(1);
    return NULL;
}

int main()
{
    pthread_t thread_id[THREADS];
    int count;
    void *result;
    int status;

    for (count = 0; count < THREADS; count++)
    {
        status = pthread_create(&thread_id[count], NULL, thread_routine, NULL);
        if (status != 0)
            err_abort(status, "Create thread");
    }

    sleep(2);

    for (count = 0; count < THREADS; count++)
    {
        status = pthread_cancel(thread_id[count]);
        if (status != 0)
            err_abort(status, "Cancel thread");

        status = pthread_join(thread_id[count], &result);
        if (status != 0)
            err_abort(status, "Join thread");

        if (result == PTHREAD_CANCELED)
            printf("Thread %d canceled\n", count);
        else
            printf("Thread %d was not canceled\n", count);
    }
}