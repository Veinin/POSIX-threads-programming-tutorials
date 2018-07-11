#ifndef BARRIER_H
#define BARRIER_H

#include <pthread.h>

typedef struct barrier_tag {
    pthread_mutex_t     mutex;
    pthread_cond_t      cv;
    int                 valid;
    int                 threshold;
    int                 counter;
    unsigned long       cycle;
} barrier_t;

#define BARRIER_VALID 0xdbcafe

#define BARRIER_INITIALIZER(cnt) \
    {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, \
    BARRIER_VALID, cnt, cnt, 0}

int barrier_init(barrier_t *barrier, int count);
int barrier_destroy(barrier_t *barrier);
int barrier_wait(barrier_t *barrier);

#endif //BARRIER_H