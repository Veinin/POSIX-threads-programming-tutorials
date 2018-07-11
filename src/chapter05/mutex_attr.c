#include <pthread.h>
#include "errors.h"

pthread_mutex_t mutex;

int main()
{
    int status;
    int pshared;
    pthread_mutexattr_t mutex_attr;

    status = pthread_mutexattr_init(&mutex_attr);
    if (status != 0)
        err_abort(status, "Init mutex attr");

    status = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    if (status != 0)
        err_abort(status, "Set pshared");

    status = pthread_mutex_init(&mutex, &mutex_attr);
    if (status != 0)
        err_abort(status, "Init mutex");

    status = pthread_mutexattr_getpshared(&mutex_attr, &pshared);
    if (status != 0)
        err_abort(status, "Get pshared");
    
    printf("pshared: %d\n", pshared);

    return 0;
}