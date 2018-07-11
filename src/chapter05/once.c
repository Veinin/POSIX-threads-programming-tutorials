#include <pthread.h>
#include "errors.h"

pthread_once_t once_block = PTHREAD_ONCE_INIT;
pthread_mutex_t mutex;

void once_init_routine(void)
{
    int status;

    status = pthread_mutex_init(&mutex, NULL);
    if (status != 0)
        err_abort(status, "Init mutex");
}

void *thread_routine(void *arg)
{
    int status;

    status = pthread_once(&once_block, once_init_routine);
    if (status != 0)
        err_abort(status, "Once init");

    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        err_abort(status, "Init mutex");

    printf("thread_toutine has locked the mutex.\n");

    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        err_abort(status, "Destroy mutex");

    return NULL;
}

int main()
{
    int status;
    pthread_t thread_id;
    
    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    status = pthread_once(&once_block, once_init_routine);
    if (status != 0)
        err_abort(status, "Once init");

    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        err_abort(status, "Init mutex");

    printf("Main has locked the mutex.\n");

    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        err_abort(status, "Destroy mutex");

    status = pthread_join(thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join thread");

    return 0;
}