#include <pthread.h>
#include <time.h>
#include "errors.h"

typedef struct my_struct_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             value;
} my_struct_t;

my_struct_t data = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};

int hibernation = 1;

void *wait_thread(void *arg)
{
    int status;

    sleep(hibernation);

    status = pthread_mutex_lock(&data.mutex);
    if (status != 0)
        err_abort(status, "Lock mutex");

    data.value = 1;

    status = pthread_cond_signal(&data.cond);
    if (status != 0)
        err_abort(status, "Signal condition");

    status = pthread_mutex_unlock(&data.mutex);
    if (status != 0)
        err_abort(status, "Unlock mutex");

    return NULL;
}

int main(int argc, char *argv[])
{
    int status;
    pthread_t wait_thread_id;
    struct timespec timeout;

    if (argc > 1)
        hibernation = atoi(argv[1]);

    status = pthread_create(&wait_thread_id, NULL, wait_thread, NULL);
    if (status != 0)
        err_abort(status, "Create wait thread");

    timeout.tv_sec = time(NULL) + 2;
    timeout.tv_nsec = 0;

    status = pthread_mutex_lock(&data.mutex);
    if (status != 0)
        err_abort(status, "Lock mutex");

    while (data.value == 0) {
        status = pthread_cond_timedwait(&data.cond, &data.mutex, &timeout);
        if (status == ETIMEDOUT) {
            printf("Condition wait time out.\n");
            break;
        } else if (status != 0)
            err_abort(status, "Wait on condition");
    }

    status = pthread_mutex_unlock(&data.mutex);
    if (status != 0)
        err_abort(status, "Unlock mutex");

    return 0;
}