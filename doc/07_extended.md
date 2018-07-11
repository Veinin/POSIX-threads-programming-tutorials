# 线程扩展

## 栅栏（Barriers）

barrier 字面意思时栅栏，barrier 是将一组成员保持在一起的一种方式，它可以实现让一组线程等待至某个状之后再全部同时执行。一个 barrier 通常被用来确保某些井行算法中的所有合作线程在任何线程可以继续运行之前到达算法中的一个特定点。

barrier 的核心是一个计数器，我们可以称之为“阈值”，即在一个 barrier 上必须等待的线程数。计时器计算着当前线程的返回数量，如果数量未达到指定点，那么之前返回的线程都必须继续等待，直到最后一个线程返回，才能开始下一步。

你可以查看源文件 `barrier.h` 、`barrier.c`、`barrier_main.c`，这是一个比较容易理解的实现。

另外 Pthreads 在 POSIX.14 草案标准（一个 “POSIX 标准子集”）中新增加了 Barriers 变量，其 API 如下：

```c
#include <pthread.h>

int pthread_barrier_init(pthread_barrier_t * restrict barrier, 
    const pthread_barrierattr_t * restrict attr, unsigned int count);

int pthread_barrier_destroy(pthread_barrier_t *barrier);

int pthread_barrier_wait(pthread_barrier_t *barrier);
```

源文件 `pthread_barrier.c` 是一个 Pthreads barrier 的简单实例。

## 读/写锁（Read-Write Lock）

读/写锁很像一个互斥量，它是阻止多个线程同时修改共享数据的另外一种方。但是不同于互斥量的是它区分读数据和写数据。一个互斥量排除所有的其他线程，而一个读/写锁如果线程不需要改变数据，则允许多个线程同时读数据。当一个线程需要更新缓存数据是，则必须以独占的方式进行，其他只读线程都不能继续占有锁。

读/写锁被用来保护经常需要读但是通常不需要修改的信息（读多写少）。当写锁被释放时，如果读数据者和写数据者同时正在等待存取，则读数据者被优先给予访问权。因为潜在地允许许多线程同时完成工作，读访问优先有利于并发。

文件 `rwlock.h` 、`rwlock.c`、`rwlock_main.c` 演示了如何使用标准的 Pthreads 互斥量和状况变量实现读写锁， 这是相对容易理解的可移植的实现。

另外，在最新版本的 `X/Open XSH5 [UNIX98]` 标准中，Pthreads 增加了读写锁的支持，读写锁的数据类型为 `pthread_rwlock_t`，如果需要静态分配该类型数据，那么可通过`PTHREAD_RWLOCK_INITIALIZER` 宏来初始化它。Pthreads 读写锁 API 如下：

```c
#include <pthread.h>

pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;

int pthread_rwlock_init(pthread_rwlock_t * restrict lock, const pthread_rwlockattr_t * restrict attr);
int pthread_rwlock_destroy(pthread_rwlock_t *lock);

int pthread_rwlock_rdlock(pthread_rwlock_t *lock);
int pthread_rwlock_timedrdlock(pthread_rwlock_t * restrict lock, const struct timespec * restrict abstime);
int pthread_rwlock_tryrdlock(pthread_rwlock_t *lock);

int pthread_rwlock_wrlock(pthread_rwlock_t *lock);
int pthread_rwlock_timedwrlock(pthread_rwlock_t * restrict lock, const struct timespec * restrict abstime);
int pthread_rwlock_trywrlock(pthread_rwlock_t *lock);

int pthread_rwlock_unlock(pthread_rwlock_t *lock);
```

你可以查看源文件 `pthread_rwlock.c`，这是简单的 Pthread 读写锁的使用实例。

## 自旋锁（Spin Locks）

自旋锁（Spinlock）也是一种锁，自旋锁在线程尝试获取它时，会在一个循环中不停等待（旋转），同时反复检查锁是否可用。由于线程始终保持活动状态且并没有执行有用的任务，因此使用这种锁时将产生一种忙碌的等待情况。

因为在一些多线程场景中我们需要避免操作系统进程的重新调度或者上下文的切换开销，所以如果线程仅仅只是短时间内被阻塞，那么使用自旋锁将是一种非常有效的方式。但是，如果你的程序需要比较长的时间保持锁的使用，那么自旋锁将会变的浪费，因为它会阻止其他线程运行。线程持有锁的时间越长，操作系统调度程序在保持锁定时中断线程的风险就越大。在这种情况下，其他线程将会不停“旋转”（反复尝试获取锁定），而持有锁的线程没有进行释放。结果将是无限期推迟，直到持有锁的线程完成并释放它。

下面是一个自旋锁的实现方式，该实现中使用了 GCC 提供的原子操作的相关函数：

```c
typedef struct spinlock_type {
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
```

你可以查看源文件 `spinlock.h` 和 `spinlock_main.c`，这是一个自旋锁的简单使用实例，演示了10个线程并发使用自旋锁修改单一数据流程。

另外，Pthreads 也提供了自旋锁的实现，其 API 定义如下：

```c
#include <pthread.h>

int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
int pthread_spin_lock(pthread_spinlock_t *lock);
int pthread_spin_trylock(pthread_spinlock_t *lock);
int pthread_spin_unlock(pthread_spinlock_t *lock);
int pthread_spin_destroy(pthread_spinlock_t *lock);
```

你可以在源文件 `pthread_spinlock.c` 中查看对自旋锁的使用实例。

## 信号量（Semaphore）

信号量是由 EW Dijkstra 在20世纪60年代后期设计的编程结构。Dijkstra 的模型是铁路运营。考虑一段铁路，其中存在单个轨道，在该轨道上一次只允许一列火车。

信号量同步此轨道上的行程。火车必须在进入单轨之前等待，直到信号量处于允许旅行的状态。当火车进入轨道时，信号量改变状态以防止其他列车进入轨道。离开这段赛道的火车必须再次改变信号量的状态，以允许另一列火车进入。

在计算机版本中，信号量似乎是一个简单的整数。线程等待许可继续，然后通过对信号量执行`P操作`来发出线程已经继续的信号。

线程必须等到信号量的值为正，然后通过从值中减去1来更改信号量的值。完成此操作后，线程执行`V操作`，通过向该值加1来更改信号量的值。这些操作必须以原子方式进行。在`P操作`中，信号量的值必须在值递减之前为正，从而产生一个值，该值保证为非负值，并且比递减之前的值小1。

### 信号量 API

```c
#include <semaphore.h>

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_destroy(sem_t *sem);
```

信号量初始化函数中，pshared的值为零，则不能在进程之间共享信号量。如果pshared的值非零，则可以在进程之间共享信号量。值 value 之名，
其中，`sem_post` 以原子方式递增sem指向的信号量。调用后，当信号量上的任何线程被阻塞时，其中一个线程被解除阻塞。
使用 `sem_wait` 来阻塞调用线程，直到sem指向的信号量计数变为大于零，然后原子地减少计数。

### 信号量解决生产者与消费者问题

生产者和消费者问题是并发编程中标准的，众所周知的一个小问题。在一个缓冲区中，分为将项目放入缓冲区生产者，从缓冲区中取出项目的消费者。

在缓冲区有可用空间之前，生产者不能在缓冲区中放置东西。在生产者写入缓冲区之前，消费者不能从缓冲区中取出东西。

你可以查看源文件 `pthread-semaphore.c`，这是一个使用信号量解决生产者、消费者问题的实例。

## 工作队列

工作队列是一组线程间分派工作的方法，创建工作队列时，可以指定需要的最大并发级别（最大线程数量）。

依据工作量的要求，线程将被开始或停止。没有发现任何请求的一个线程将等待一段时间后终止。最优的时间段取决于在你的系统上创建一个新线程的开销、维护一个不做任何工作的线程的系统资源的开销，以及你将再次需要线程的可能性。

源文件 `workq.h` 、`workq.c` 和 `workq_main.c` 显示了一个工作队列管理器的实现。