#include <pthread.h>
#include "errors.h"

static int counter;

void *thread_routine(void *arg)
{
    printf("thread_routine starting\n");
    for (counter == 0; ; counter++)
    {
        if ((counter % 1000) == 0)
        {
            printf("calling testcancel\n");
            pthread_testcancel();
        }
    }
}

int main()
{
    pthread_t thread_id;
    void *result;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    sleep(2);

    printf("callling cancel\n");

    status = pthread_cancel(thread_id);
    if (status != 0)
        err_abort(status, "Cancel thread");

    printf("calling join\n");

    status = pthread_join(thread_id, &result);
    if (status != 0)
        err_abort(status, "Join thread");

    if (result == PTHREAD_CANCELED)
        printf("Thread canceled at iteration %d\n", counter);
    else
        printf("Thread was not canceled\n");

    return 0;
}