#include <pthread.h>
#include "errors.h"

typedef struct my_struct_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             value;
} my_struct_t;

my_struct_t data = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};

int main(int argc, char const *argv[])
{
    return 0;
}