#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main()
{
    pthread_t thread;
    int status;

    status = pthread_join(thread, NULL);
    if (status != 0)
        fprintf(stderr, "error %d: %s\n", status, strerror(status));

    return status;
}