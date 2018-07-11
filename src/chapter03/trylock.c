#include <pthread.h>
#include "errors.h"

#define SPIN 10000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long counter;
time_t end_time;

void *counter_thread(void *arg)
{
    int status;
    int spin;

    while (time(NULL) < end_time) 
    {
        status = pthread_mutex_lock(&mutex);
        if (status != 0)
            err_abort(status, "Lock mutex");

        for (spin = 0; spin < SPIN; spin++)
            counter++;

        status = pthread_mutex_unlock(&mutex);
        if (status != 0)
            err_abort(status, "Unlock mutex");

        sleep(1);
    }

    printf("Counter is %ld\n", counter);
    return NULL;
}

void *monitor_thread(void *arg)
{
    int status;
    int misses = 0;

    while (time(NULL) < end_time)
    {
        sleep(3);
        status = pthread_mutex_trylock(&mutex);
        if (status != EBUSY)
        {
            if (status != 0)
                err_abort(status, "Trylock mutex");
            
            printf("Counter is %ld\n", counter/SPIN);

            status = pthread_mutex_unlock(&mutex);
            if (status != 0)
                err_abort(status, "Unlock mutex");
        } else
            misses++;
    }

    printf("Monitor thread missed update %d times.\n", misses);
    return NULL;
}

int main()
{
    int status;
    pthread_t counter_thread_id;
    pthread_t monitor_thread_id;

    end_time = time(NULL) + 60;

    status = pthread_create(&counter_thread_id, NULL, counter_thread, NULL);
    if (status != 0)
        err_abort(status, "Create counter thread");

    status = pthread_create(&monitor_thread_id, NULL, monitor_thread, NULL);
    if (status != 0)
        err_abort(status, "Create monitor thread");

    status = pthread_join(counter_thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join counter thread");

    status = pthread_join(monitor_thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join monitor thread");

    return 0;
}