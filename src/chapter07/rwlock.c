#include "errors.h"
#include "rwlock.h"

int rwl_init(rwlock_t *rwl)
{
    int status;
    rwl->r_active = rwl->w_active = 0;
    rwl->r_wait = rwl->w_wait = 0;

    status = pthread_mutex_init(&rwl->mutex, NULL);
    if (status != 0)
        return status;

    status = pthread_cond_init(&rwl->read, NULL);
    if (status != 0) {
        pthread_mutex_destroy(&rwl->mutex);
        return status;
    }

    status = pthread_cond_init(&rwl->write, NULL);
    if (status != 0) {
        pthread_mutex_destroy(&rwl->mutex);
        pthread_cond_destroy(&rwl->read);
        return status;
    }

    rwl->valid = RWLOCK_VALID;
    return 0;
}

int rwl_destroy(rwlock_t *rwl)
{
    int status, status1, status2;

    if (rwl->valid != RWLOCK_VALID)
        return EINVAL;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->r_active > 0 || rwl->w_active) {
        pthread_mutex_unlock(&rwl->mutex);
        return EBUSY;
    }

    if (rwl->r_wait > 0 || rwl->w_wait > 0) {
        pthread_mutex_unlock(&rwl->mutex);
        return EBUSY;
    }

    rwl->valid = 0;
    
    status = pthread_mutex_unlock(&rwl->mutex);
    if (status != 0)
        return status;

    status = pthread_mutex_destroy(&rwl->mutex);
    status1 = pthread_cond_destroy(&rwl->read);
    status2 = pthread_cond_destroy(&rwl->write);

    return (status != 0 ? status : (status1 != 0 ? status1 : status2));
}

static void rwl_readcleanup(void *arg)
{
    rwlock_t *rwl = (rwlock_t *)arg;

    rwl->r_wait--;
    pthread_mutex_unlock(&rwl->mutex);
}

static void rwl_writecleanup(void *arg)
{
    rwlock_t *rwl = (rwlock_t *)arg;

    rwl->w_wait--;
    pthread_mutex_unlock(&rwl->mutex);
}

int rwl_readlock(rwlock_t *rwl)
{
    int status;

    if (rwl->valid != RWLOCK_VALID)
        return EINVAL;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->w_active) {
        rwl->r_wait++;
        pthread_cleanup_push(rwl_readcleanup, (void *)rwl);
        while (rwl->w_active) {
            status = pthread_cond_wait(&rwl->read, &rwl->mutex);
            if (status != 0)
                break;
        }
        pthread_cleanup_pop(0);
    }

    if (status == 0)
        rwl->r_active++;
    
    pthread_mutex_unlock(&rwl->mutex);
    return status;
}

int rwl_readtrylock(rwlock_t *rwl)
{
    int status, status2;

    if (rwl->valid != RWLOCK_VALID)
        return EINVAL;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->w_active)
        status = EBUSY;
    else
        rwl->r_active++;

    status2 = pthread_mutex_unlock(&rwl->mutex);
    return (status2 != 0 ? status2 : status);
}

int rwl_readunlock(rwlock_t *rwl)
{
    int status, status2;

    if (rwl->valid != RWLOCK_VALID)
        return EINVAL;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    rwl->r_active--;

    if (rwl->r_active == 0 && rwl->w_wait > 0)
        status = pthread_cond_signal(&rwl->write);

    status2 = pthread_mutex_unlock(&rwl->mutex);
    return (status2 == 0 ? status : status2);
}

int rwl_writelock(rwlock_t *rwl)
{
    int status;

    if (rwl->valid != RWLOCK_VALID)
        return EINVAL;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->w_active || rwl->r_active > 0) {
        rwl->w_wait++;
        pthread_cleanup_push(rwl_writecleanup, (void *)rwl);
        while (rwl->w_active || rwl->r_active > 0) {
            status = pthread_cond_wait(&rwl->write, &rwl->mutex);
            if (status != 0)
                break;
        }
        pthread_cleanup_pop(0);
        rwl->w_wait--;
    }

    if (status == 0)
        rwl->w_active = 1;

    pthread_mutex_unlock(&rwl->mutex);

    return status;
}

int rwl_writetrylock(rwlock_t *rwl)
{
    int status, status2;

    if (rwl->valid != RWLOCK_VALID)
        return EINVAL;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    if (rwl->w_active || rwl->r_active > 0)
        status = EBUSY;
    else
        rwl->w_active = 1;

    status2 = pthread_mutex_unlock(&rwl->mutex);
    return (status != 0 ? status : status2);
}

int rwl_writeunlock(rwlock_t *rwl)
{
    int status;

    if (rwl->valid != RWLOCK_VALID)
        return EINVAL;

    status = pthread_mutex_lock(&rwl->mutex);
    if (status != 0)
        return status;

    rwl->w_active = 0;
    if (rwl->r_wait > 0) {
        status = pthread_cond_broadcast(&rwl->read);
        if (status != 0) {
            pthread_mutex_unlock(&rwl->mutex);
            return status;
        }
    } else if (rwl->w_wait > 0) {
        status = pthread_cond_signal(&rwl->write);
        if (status != 0) {
            pthread_mutex_unlock(&rwl->mutex);
            return status;
        }
    }

    status = pthread_mutex_unlock(&rwl->mutex);
    return status;
}