#ifndef WORKQ_H
#define WORKQ_H

#include <pthread.h>

typedef struct workq_ele_tag {
    struct workq_ele_tag    *next;
    void                    *data;
} workq_ele_t;

typedef struct workq_tag {
    pthread_mutex_t     mutex;
    pthread_cond_t      cv;
    pthread_attr_t      attr;
    workq_ele_t         *first, *last;
    int                 valid;
    int                 quit;
    int                 parallelism;
    int                 counter;
    int                 idle;
    void                (*engine)(void *);
} workq_t;

#define WORKQ_VALID 0xdec2018

int workq_init(workq_t *wq, int threads, void (*engine)(void *));
int workq_destroy(workq_t *wq);
int workq_add(workq_t *wq, void *data);

#endif