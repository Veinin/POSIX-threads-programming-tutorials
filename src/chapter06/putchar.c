#include <pthread.h>
#include "errors.h"

void *lock_routine(void *arg)
{
    char *pointer;

    flockfile(stdout);
    for (pointer = arg; *pointer != '\0'; pointer++) {
        putchar_unlocked(*pointer);
        sleep(1);
    }
    funlockfile(stdout);
    return NULL;
}

void *unlock_routine(void *arg)
{
    char *pointer;

    for (pointer = arg; *pointer != '\0'; pointer++) {
        putchar_unlocked(*pointer);
        sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t thread1, thread2, thread3;
    int flock_flag = 1;
    int status;
    void *(*thread_func)(void *);

    if (argc > 1)
        flock_flag = atoi(argv[1]);

    if (flock_flag)
        thread_func = lock_routine;
    else
        thread_func = unlock_routine;

    status = pthread_create(&thread1, NULL, thread_func, "this is thread 1\n");
    if (status != 0)
        err_abort(status, "Create thread");

    status = pthread_create(&thread2, NULL, thread_func, "this is thread 2\n");
    if (status != 0)
        err_abort(status, "Create thread");

    status = pthread_create(&thread3, NULL, thread_func, "this is thread 3\n");
    if (status != 0)
        err_abort(status, "Create thread");

    pthread_exit(NULL);
}