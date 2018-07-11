#include <pthread.h>
#include "errors.h"

#define THREADS 3

void *prompt_routine(void *arg)
{
    int prompt = *(int*)arg;
    char *string;
    int len;

    string = (char*)malloc(128);
    if (string == NULL)
        errno_abort("Alloc string");

    flockfile(stdin);
    flockfile(stdout);

    printf("Thread %d> ", prompt);
    if (fgets(string, 127, stdin) == NULL)
        string[0] = '\0';
    else {
        len = strlen(string);
        if (len > 0 && string[len - 1] == '\n')
            string[len -1] = '\0';
    }

    funlockfile(stdout);
    funlockfile(stdin);
    return (void*)string;
}

int main()
{
    pthread_t threads[THREADS];
    int count;
    void *string;
    int status;

    for (count = 0; count < THREADS; count++) {
        status =  pthread_create(&threads[count], NULL, prompt_routine, &count);
        if (status != 0)
            err_abort(status, "Create thread");
    }

    for (count = 0; count < THREADS; count++) {
        status = pthread_join(threads[count], &string);
        if (status != 0)
            err_abort(status, "Join thread");

        printf("Thread %d: \"%s\"\n", count, (char*)string);
        free(string);
    }

    return 0;
}