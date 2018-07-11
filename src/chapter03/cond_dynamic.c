#include <pthread.h>
#include "errors.h"

typedef struct my_struct_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             value;
} my_struct_t;

int main(int argc, char const *argv[])
{
    my_struct_t *data;
    int status;
 
    data = malloc(sizeof(my_struct_t));
    if (data == NULL)
        errno_abort("Allocate structure");

    status = pthread_mutex_init(&data->mutex, NULL);
    if (status != 0)
        err_abort(status, "Init mutex");

    status = pthread_cond_init(&data->cond, NULL);
    if (status != 0)
        err_abort(status, "Init condition");

    status = pthread_cond_destroy(&data->cond);
    if (status != 0)
        err_abort(status, "Destroy condition");

    status = pthread_mutex_destroy(&data->mutex);
    if (status != 0)
        err_abort(status, "Destroy mutex");

    free(data);

    return status;
}
