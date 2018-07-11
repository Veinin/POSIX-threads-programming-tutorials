#include <pthread.h>
#include "errors.h"

pthread_cond_t cond;

int main()
{
    int status;
    pthread_condattr_t cond_attr;

    status = pthread_condattr_init(&cond_attr);
    if (status != 0)
        err_abort(status, "Create attr");

    status = pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_PRIVATE);
    if (status != 0)
        err_abort(status, "Set pshared");

    status = pthread_cond_init(&cond, &cond_attr);
    if (status != 0)
        err_abort(status, "Init cond");

    return 0;
}