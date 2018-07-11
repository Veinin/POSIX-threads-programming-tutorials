#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include "errors.h"

pid_t self_pid;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void fork_prepare(void)
{
    int status;

    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        err_abort(status, "Lock in prepare handler");
}

void fork_parent(void)
{
    int status;

    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        err_abort(status, "Unlock in parent handler");
}

void fork_child(void)
{
    int status;

    self_pid = getpid();
    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        err_abort(status, "Unlock in child handler");
}

void *thread_routine(void *arg)
{
    pid_t child_pid;
    int status;

    child_pid = fork();
    if (child_pid == (pid_t)-1)
        errno_abort("Fork");

    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        err_abort(status, "Lock in child");

    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        err_abort(status, "Unlock in child");

    printf("After fork: %d (%d)\n", child_pid, self_pid);

    if (child_pid != 0) {
        if ((pid_t)-1 == waitpid(child_pid, (int *)0, 0))
            errno_abort("Wait for child");
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t fork_thread;
    int atfork_flag = 1;
    int status;

    if (argc > 1)
        atfork_flag = atoi(argv[1]);

    if (atfork_flag) {
        status = pthread_atfork(fork_prepare, fork_parent, fork_child);
        if (status != 0)
            err_abort(status, "Register fork handlers");
    }

    self_pid = getpid();
    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        err_abort(status, "Lock mutex");

    status = pthread_create(&fork_thread, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    sleep(5);

    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        err_abort(status, "Unlock mutex");

    status = pthread_join(fork_thread, NULL);
    if (status != 0)
        err_abort(status, "Join thread");

    return 0;
}