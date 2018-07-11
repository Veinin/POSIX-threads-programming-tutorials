#include <time.h>
#include "workq.h"
#include "errors.h"

int workq_init(workq_t *wq, int threads, void (*engine)(void *arg))
{
    int status;

    status = pthread_attr_init(&wq->attr);
    if (status != 0)
        return status;

    status = pthread_attr_setdetachstate(&wq->attr, PTHREAD_CREATE_DETACHED);
    if (status != 0) {
        pthread_attr_destroy(&wq->attr);
        return status;
    }

    status = pthread_mutex_init(&wq->mutex, NULL);
    if (status != 0) {
        pthread_attr_destroy(&wq->attr);
        return status;
    }

    status = pthread_cond_init(&wq->cv, NULL);
    if (status != 0) {
        pthread_mutex_destroy(&wq->mutex);
        pthread_attr_destroy(&wq->attr);
        return status;
    }

    wq->quit = 0;
    wq->first = wq->last = NULL;
    wq->parallelism = threads;
    wq->counter = 0;
    wq->idle = 0;
    wq->engine = engine;
    wq->valid = WORKQ_VALID;

    return 0;
}

int workq_destroy(workq_t *wq)
{
    int status, status1, status2;

    if (wq->valid != WORKQ_VALID)
        return EINVAL;

    status = pthread_mutex_lock(&wq->mutex);
    if (status != 0)
        return status;

    wq->valid = 0;

    if (wq->counter > 0) {
        wq->quit = 1;
        if (wq->idle > 0) {
            status = pthread_cond_broadcast(&wq->cv);
            if (status != 0) {
                pthread_mutex_unlock(&wq->mutex);
                return status;
            }
        }

        while (wq->counter > 0) {
            status = pthread_cond_wait(&wq->cv, &wq->mutex);
            if (status != 0) {
                pthread_mutex_unlock(&wq->mutex);
                return status;
            }
        }
    }

    status = pthread_mutex_unlock(&wq->mutex);
    if (status != 0)
        return status;

    status = pthread_mutex_destroy(&wq->mutex);
    status1 = pthread_cond_destroy(&wq->cv);
    status2 = pthread_attr_destroy(&wq->attr);
    return (status ? status : (status1 ? status1 : status2));
}

static void *workq_server(void *arg)
{
    struct timespec timeout;
    workq_t *wq = (workq_t *)arg;
    workq_ele_t *we;
    int status, timedout;

    printf("A worker is starting\n");
    status = pthread_mutex_lock(&wq->mutex);
    if (status != 0)
        return NULL;

    while (1) {
        timedout = 0;
        printf("Worker waiting for work\n");
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_sec += 2;

        while (wq->first == NULL && !wq->quit) {
            status = pthread_cond_timedwait(&wq->cv, &wq->mutex, &timeout);
            if (status == ETIMEDOUT) {
                printf("Worker wait timed out\n");
                timedout = 1;
                break;
            } else if (status != 0) {
                printf("Worker wait failed, %d (%s)\n", status, strerror(status));
                wq->counter--;
                pthread_mutex_unlock(&wq->mutex);
                return NULL;
            }
        }

        printf("Work queue: 0x%p, quit: %d\n", wq->first, wq->quit);
        we = wq->first;

        if (we != NULL) {
            wq->first = we->next;
            if (wq->last == we)
                wq->last = NULL;
            
            status = pthread_mutex_unlock(&wq->mutex);
            if (status != 0)
                return NULL;

            printf("Worker calling engine\n");
            wq->engine(we->data);
            free(we);

            status = pthread_mutex_lock(&wq->mutex);
            if (status != 0)
                return NULL;
        }

        if (wq->first == NULL && wq->quit) {
            printf("Worker shutting down\n");
            wq->counter--;

            if (wq->counter == 0)
                pthread_cond_broadcast(&wq->cv);

            pthread_mutex_unlock(&wq->mutex);
            return NULL;
        }

        if (wq->first == NULL && timedout) {
            printf("engine terminating due to timeout.\n");
            wq->counter--;
            break;
        }
    }

    pthread_mutex_unlock(&wq->mutex);
    printf("worker exiting\n");
    return NULL;
}

int workq_add(workq_t *wq, void *element)
{
    workq_ele_t *item;
    pthread_t id;
    int status;

    if (wq->valid != WORKQ_VALID)
        return EINVAL;

    item = (workq_ele_t *)malloc(sizeof(workq_ele_t));
    if (item == NULL)
        return ENOMEM;

    item->data = element;
    item->next = NULL;

    status = pthread_mutex_lock(&wq->mutex);
    if (status != 0) {
        free(item);
        return status;
    }

    if (wq->first == NULL)
        wq->first = item;
    else
        wq->last->next = item;
    wq->last = item;

    if (wq->idle > 0) {
        status = pthread_cond_signal(&wq->cv);
        if (status != 0) {
            pthread_mutex_unlock(&wq->mutex);
            return status;
        }
    } else if(wq->counter < wq->parallelism) {
        printf("Creating new worker\n");
        status = pthread_create(&id, &wq->attr, workq_server, (void*)wq);
        if (status != 0) {
            pthread_mutex_unlock(&wq->mutex);
            return status;
        }
        wq->counter++;
    }

    pthread_mutex_unlock(&wq->mutex);
    return 0;
}