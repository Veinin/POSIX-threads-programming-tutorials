#include <pthread.h>
#include "errors.h"

typedef struct private_tag {
    pthread_t   thread_id;
    char        *string;
} private_t;

pthread_key_t identity_key;
pthread_mutex_t identity_key_mutex = PTHREAD_MUTEX_INITIALIZER;
long identity_key_counter = 0;

void identity_key_destructor(void *value)
{
    private_t *private = (private_t*)value;
    int status;

    printf("thread \"%s\" exiting...\n", private->string);
    free(value);

    status = pthread_mutex_lock(&identity_key_mutex);
    if (status != 0)
        err_abort(status, "Lock key mutex");

    identity_key_counter--;
    if (identity_key_counter <= 0) {
        status = pthread_key_delete(identity_key);
        if (status != 0)
            err_abort(status, "Delete key");
        printf("key delete...\n");
    }

    status = pthread_mutex_unlock(&identity_key_mutex);
    if (status != 0)
            err_abort(status, "Unlock key mutex");
}

void *identity_key_get(void)
{
    void *value;
    int status;

    value = pthread_getspecific(identity_key);
    if (value == NULL) {
        value = malloc(sizeof(private_t));
        if (value == NULL)
            errno_abort("Allocate key value");

        status = pthread_setspecific(identity_key, value);
        if (status != 0)
            err_abort(status, "Set TSD");
    }

    return value;
}

void *thread_routine(void *arg)
{
    private_t *value;

    value = (private_t*) identity_key_get();
    value->thread_id = pthread_self();
    value->string = (char*)arg;
    printf("thread \"%s\" starting...\n", value->string);
    sleep(2);
    return NULL;
}

int main()
{
    pthread_t thread_1, thread_2;
    private_t *value;
    int status;

    status = pthread_key_create(&identity_key, identity_key_destructor);
    if (status != 0)
            err_abort(status, "Create key");

    identity_key_counter = 3;

    value = (private_t*)identity_key_get();
    value->thread_id = pthread_self();
    value->string = "Main thread";

    status = pthread_create(&thread_1, NULL, thread_routine, "thread 1");
    if (status != 0)
        err_abort(status, "Create thread 1");

    status = pthread_create(&thread_2, NULL, thread_routine, "thread 2");
    if (status != 0)
        err_abort(status, "Create thread 2");

    pthread_exit(NULL);
}