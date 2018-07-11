#include <limits.h>
#include <pthread.h>
#include "errors.h"

void *thread_routine(void *arg)
{
    printf("The thread is here\n");
    return NULL;
}

int main()
{
    pthread_t thread_id;
    pthread_attr_t thread_attr;
    struct sched_param thread_param;
    size_t stack_size;
    int status;

    status = pthread_attr_init(&thread_attr);
    if (status != 0)
        err_abort(status, "Create attr");

    status = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (status != 0)
        err_abort(status, "Set detach");

#ifdef _POSIX_THREAD_ATTR_STACKSIZE
    status = pthread_attr_getstacksize(&thread_attr, &stack_size);
    if (status != 0)
        err_abort(status, "Get stack size");

    printf("Default stack size is %zu; minimum is %u\n", stack_size, PTHREAD_STACK_MIN);

    status = pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN*2);
    if (status != 0)
        err_abort(status, "Set stack size");
#endif

    status = pthread_create(&thread_id, &thread_attr, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    printf("Main exiting\n");
    pthread_exit(NULL);
    return 0;
}