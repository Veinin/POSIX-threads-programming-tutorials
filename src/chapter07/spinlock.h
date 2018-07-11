#ifndef SPINLOCK_H
#define SPINLOCK_H

typedef struct spinlock_tag {
    int lock;
} spinlock_t;

static inline void spinlock_init(spinlock_t *sl)
{
    sl->lock = 0;
}

static inline void spinlock_lock(spinlock_t *sl)
{
    while(__sync_lock_test_and_set(&sl->lock, 1)) {}
}

static inline int spinlock_trylock(spinlock_t *sl)
{
    return __sync_lock_test_and_set(&sl->lock, 1) == 0;
}

static inline void spinlock_unlock(spinlock_t *sl)
{
    __sync_lock_release(&sl->lock);
}

static inline void spinlock_destroy(spinlock_t *sl)
{
    (void) sl;
}

#endif //SPINLOCK_H