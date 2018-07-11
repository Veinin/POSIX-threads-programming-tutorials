# 同步

## 不变量、临界区和谓词

不变量（invariant) 是由程序作出的假设， 特别是有关变量组间关系的假设。不变量可能会被破坏， 而且会经常被独立的代码段破坏。
临界区（critical section）有时称为“串行区域”，是指影响共享数据的代码段，临界区总能够对应到一个数据不变量。例如，你从队列中删除数据时， 你可以将删除数据的代码视为临界区。
谓词（Predicate) 是描述代码所需不变量的状态的语句。在英语中，谓词可以是如“队列为空”、 “资源可用” 之类的陈述。

## 互斥量

大部分多线程程序需要在线程间共享数据。如果两个线程同时访问共享数据就可能会有问，因为一个线程可能在另一个线程修改共享数据的过程中使用该数据，并认为共享数据保持末变。
使线程同步最通用和常用的方法就是确保对相同数据的内存访问“互斥地”进行，即一次只能允许一个线程写数据，其他线程必须等待。
同步不仅仅在修改数据时重要， 当线程需要读取其他线程写入的数据时，而且数据写入的顺序也有影响时，同样需要同步。

### 创建和销毁互斥量

Pthreads 的互斥量用 `pthread_mutex_t` 类型的变量来表示。不能拷贝互斥量，拷贝的互斥量是不确定的，但可以拷贝指向互斥量的指针。

大部分时间互斥量在函数体外，如果有其他文件使用互斥量，声明为外部类型，如果仅在本文将内使用，则将其声明为静态类型。可以使用宏 `PTHREAD_WTEX_INZTIALIZER` 来声明具有默认属性的静态互斥量，静态初始化的互斥量不需要主动释放。

下面程序演示了一个静态创建互斥量的程序，该程序 `main` 函数为空，不会产生任何结果。

```c
#include <pthread.h>
#include "errors.h"

typedef struct my_struct_tag {
    pthread_mutex_t mutex;
    int             value;
} my_struct_t;

my_struct_t data = {PTHREAD_MUTEX_INITIALIZER, 0};

int main()
{
    return 0;
}
```

如果要初始化一个非缺省属性的互斥量， 必须使用动态初始化。如当使用 `malloc` 动态分配一个包含互斥量的数据结构时，应该使用 `pthread_nutex_init` 调用来动态的初始化互斥量。当不需要互斥量时，应该调用 `pthread_mutex_destory` 来释放它。另外，如果想保证每个互斥量在使用前被初始化，而且只被初始化一次。可以在创建任何线程之前初始化它，如通过调用 `pthread_once`。

```c
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

下面程序演示了动态地初始化一个互斥量：

```c
#include <pthread.h>
#include "errors.h"

typedef struct my_struct_tag {
    pthread_mutex_t mutex;
    int             value;
} my_struct_t;

int main()
{
    my_struct_t *data;
    int status;

    data = malloc(sizeof(my_struct_t));
    if (data == NULL)
        errno_abort("Allocate structure");

    status = pthread_mutex_init(&data->mutex, NULL);
    if (status != 0)
        err_abort(status, "Init mutex");

    status = pthread_mutex_destroy(&data->mutex);
    if (status != 0)
        err_abort(status, "Destroy mutex");

    free(data);

    return status;
}
```

### 加锁和解锁互斥量

最简单的情况下使用互斥量通过调用 `pthread_mutex_lock` 或 `pthread_mutex_trylock` 锁住互斥量，处理共享数据，然后调用 `pthread_mutex_unlock` 解锁互斥量。为确保线程能够读取一组变量的一致的值，需要在任何读写这些变量的代码段周围锁住互斥量。
当调用线程己经锁住互斥量之后，就不能再加锁一个线程己经锁住互斥量之后，试图这样做的结果可能是返回错误(EDEADLK)，或者可能陷入“自死锁”，使线程永远等待下去。同样，你也不能解锁一个已经解锁的互斥量，不能解锁一个由其他线程锁住的互斥量。

```c
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

下面程序 alarm_mutex.c 是 alarm_thread.c 的一个改进版本，该程序效果如下：

- 所有的闹钟按时间顺序存储在一个链表结构 `alarm_list` 中。
- 互斥量 `alarm_mutex` 负责协调对闹铃请求列表 `alarm_list` 的头节点的访问。
- 主线程，获取闹钟请求，将去按时间顺序插入到 `alarm_list` 中。
- 子线程，检查最新的闹铃列表，如果列表为空，则并阻塞住一段时间（1秒），解锁互斥量，以便主线程添加新的闹铃请求。否则获取下一个请求的差值，阻塞指定时间后，产生闹铃。

```c
#include <pthread.h>
#include <time.h>
#include "errors.h"

typedef struct alarm_tag {
    struct alarm_tag    *link;
    int                 seconds;
    time_t              time;
    char                message[64];
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
alarm_t *alarm_list = NULL;

void *alarm_thread(void *arg)
{
    alarm_t *alarm;
    int sleep_time;
    time_t now;
    int status;

    while(1) {
        status = pthread_mutex_lock(&alarm_mutex);
        if (status != 0)
            err_abort(status, "Lock mutex");

        alarm = alarm_list;

        if (alarm == NULL)
            sleep_time = 1;
        else {
            alarm_list = alarm_list->link;
            now = time(NULL);
            if (alarm->time <= now)
                sleep_time = 0;
            else
                sleep_time = alarm->time - now;

        status = pthread_mutex_unlock(&alarm_mutex);
        if (status != 0)
            err_abort(status, "Unlock mutex");

        if (sleep_time > 0)
            sleep(sleep_time);
        else
            sched_yield();

        if (alarm != NULL) {
            printf("(%d) %s\n", alarm->seconds, alarm->message);
            free(alarm);
        }
    }
}

int main()
{
    int status;
    char line[128];
    alarm_t *alarm, **last, *next;
    pthread_t thread;

    status = pthread_create(&thread, NULL, alarm_thread, NULL);
    if (status != 0)
        err_abort(status, "Create alarm thread");

    while (1) {
        printf("Alarm> ");

        if (fgets(line, sizeof(line), stdin) == NULL)
            exit(0);

        if (strlen(line) <= 1)
            continue;

        alarm = (alarm_t*)malloc(sizeof(alarm_t));
        if (alarm == NULL)
            errno_abort("Allocate alarm");

        if (sscanf(line, "%d %64[^\n]", &alarm->seconds, alarm->message) < 2) {
            fprintf(stderr, "Bad command\n");
            free(alarm);
        } else {
            status = pthread_mutex_lock(&alarm_mutex);
            if (status != 0)
                err_abort(status, "Lock mutex");

            alarm->time = time(NULL) + alarm->seconds;

            last = &alarm_list;
            next = *last;
            while (next != NULL) {
                if (next->time >= alarm->time) {
                    alarm->link = next;
                    *last = alarm;
                    break;
                }
                last = &next->link;
                next = next->link;
            }

            if (next == NULL) {
                *last = alarm;
                alarm->link = NULL;
            }

            status = pthread_mutex_unlock(&alarm_mutex);
            if (status != 0)
                err_abort(status, "Unlock mutex");
        }
    }
}
```

该实例具有占用更少资源的优势，但它的响应性能不够。一旦 `alarm_thread` 线程从列表中接收了一个闹铃请求，它就进入睡眠直到闹铃到期。当它发现列表中没有闹铃请求时，也会睡眠 1 秒，以允许主线程接收新的用户请求。 当 `alarm_thread` 线程睡眠时，直到它从睡眠中返回，它都不能注意到由主线程添加到请求列表中的任何闹铃请求。这种情况下最好的办法是使用条件变量来通知共享数据的状态变化（后面章节内容）。

### 非阻塞式互斥量锁

当调用 `pthread_mutex_lock` 加锁互斥量时，如果此时互斥量己经被锁住，则调用线程将被阻塞。通常这是你希望的结果，但有时你可能希望如果互斥量己被锁住，则执行另外的代码路线，你的程序可能做其他一些有益的工作而不仅仅是等待。为此，Pthreads 提供了 `pthread_mutex_trylock` 函数，当调用互斥量己被锁住时调用该函数将返回错误代码 `EBUSY`。

下列实例程序 `trylock.c` 使用 `pthread_mutex_trylock` 函数来间歇性地报告计数器的值， 不过仅当它对计数器的访问与计数线程没有发生冲突时才报告：

```c
#include <pthread.h>
#include "errors.h"

#define SPIN 10000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long counter;
time_t end_time;

void *counter_thread(void *arg)
{
    int status;
    int spin;

    while (time(NULL) < end_time) 
    {
        status = pthread_mutex_lock(&mutex);
        if (status != 0)
            err_abort(status, "Lock mutex");

        for (spin = 0; spin < SPIN; spin++)
            counter++;

        status = pthread_mutex_unlock(&mutex);
        if (status != 0)
            err_abort(status, "Unlock mutex");

        sleep(1);
    }

    printf("Counter is %ld\n", counter);
    return NULL;
}

void *monitor_thread(void *arg)
{
    int status;
    int misses = 0;

    while (time(NULL) < end_time)
    {
        sleep(3);
        status = pthread_mutex_trylock(&mutex);
        if (status != EBUSY)
        {
            if (status != 0)
                err_abort(status, "Trylock mutex");

            printf("Counter is %ld\n", counter/SPIN);

            status = pthread_mutex_unlock(&mutex);
            if (status != 0)
                err_abort(status, "Unlock mutex");
        } else
            misses++;
    }

    printf("Monitor thread missed update %d times.\n", misses);
    return NULL;
}

int main()
{
    int status;
    pthread_t counter_thread_id;
    pthread_t monitor_thread_id;

    end_time = time(NULL) + 60;

    status = pthread_create(&counter_thread_id, NULL, counter_thread, NULL);
    if (status != 0)
        err_abort(status, "Create counter thread");

    status = pthread_create(&monitor_thread_id, NULL, monitor_thread, NULL);
    if (status != 0)
        err_abort(status, "Create monitor thread");

    status = pthread_join(counter_thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join counter thread");

    status = pthread_join(monitor_thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join monitor thread");

    return 0;
}
```

### 多个互斥量与死锁

有时，一个互斥量是不够的，特别是当你的代码需要跨越软件体系内部的界限时。例如，当多个线程同时访问一个队列结构时，你需要两个互斥量，一个用来保护队列头，一个用来保护队列元素内的数据。当为多线程建立一个树型结构时，你可能需要为每个节点设置一个互斥量。

使用多个互斥量会导致复杂度的增加。最坏的情况就是死锁的发生，即两个线程分别锁住了一个互斥量而等待对方的互斥量。一个线程锁住了互斥量 A 后，加锁互斥量 B；同时另一个线程锁住了 B 而等待互斥量 A，则你的代码就产生了经典的死锁现象。

| 第一个线程 | 第二个线程 |
|---|---|
| pthread_mutex_lock(&mutex_a) | pthread_mutex_lock(&mutex_b) |
| pthread_mutex_lock(&mutex_b) | pthread_mutex_lock(&mutex_a) |

针对死锁，考虑以下两种通用的解决方法：

- 固定加锁顺序。所有需要同时加锁互斥量A和互斥量B的代码，必须首先加锁互斥量A，然后锁互斥量B。
- 试加锁和回退。在锁住某个集合中的第一个互斥量后，使用以 `pthread_mutex_trylock` 来加锁集合中的其他互斥量，如果失败则将集合中所有己加锁互斥量释放，并重新锁。
- 如果代码不变量允许先释放互斥量 1,然后再加锁互斥量 2,就可以避免同时拥有两个互斥量的需要。但是，如果存在被破坏的不变置需要锁住不变量 1，则互斥量 1 就不能被释放，直到不变量被恢复为止。在这种情况下， 你应该考虑使用回退（或者试锁-回退 ）算法。

以下程序 `backoff.c` 演示了如何使用回退算法避免互斥量死锁。程序建立了两个线程线程，一个运行函数 `lock_forward`,一个个运行函数 `lock_backward`。程序每次循环都会试图锁住三个互斥量，`lock_forward` 依次锁住互斥量1、2、3，`lock_backward`则按相反顺序加锁互斥量：

```c
#include <pthread.h>
#include "errors.h"

#define ITERATIONS 10

pthread_mutex_t mutex[3] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER};

int backoff = 1;
int yield_flag = 0;

void *lock_forward(void *arg)
{
    int i, iterate, backoffs;
    int status;

    for (iterate = 0; iterate < ITERATIONS; iterate++)
    {
        backoffs = 0;

        for (i = 0; i < 3; i++)
        {
            if (i == 0)
            {
                status = pthread_mutex_lock(&mutex[i]);
                if (status != 0)
                    err_abort(status, "First lock");
                printf("forward lock got %d\n", i);
            }
            else
            {
                if (backoff)
                    status = pthread_mutex_trylock(&mutex[i]);
                else
                    status = pthread_mutex_lock(&mutex[i]);

                if (status == EBUSY)
                {
                    backoffs++;
                    printf("forward locker backing of at %d\n", i);
                    for (; i >= 0; i--)
                    {
                        status = pthread_mutex_unlock(&mutex[i]);
                        if (status != 0)
                            err_abort(status, "Backoff");
                    }
                }
                else
                {
                    if (status != 0)
                        err_abort(status, "Lock mutex");
                    printf("forward locker got %d\n", i);
                }
            }

            if (yield_flag)
            {
                if (yield_flag > 0)
                    sched_yield();
                else
                    sleep(1);
            }
        }

        printf("lock forward got all locks, %d backoffs\n", backoffs);
        pthread_mutex_unlock(&mutex[0]);
        pthread_mutex_unlock(&mutex[1]);
        pthread_mutex_unlock(&mutex[2]);
        sched_yield();
    }

    return NULL;
}

void *lock_backward(void *arg)
{
    int i, iterate, backoffs;
    int status;

    for (iterate = 0; iterate < ITERATIONS; iterate++)
    {
        backoffs = 0;

        for (i = 2; i >= 0; i--)
        {
            if (i == 2)
            {
                status = pthread_mutex_lock(&mutex[i]);
                if (status != 0)
                    err_abort(status, "First lock");
                printf("backward lock got %d\n", i);
            }
            else
            {
                if (backoff)
                    status = pthread_mutex_trylock(&mutex[i]);
                else
                    status = pthread_mutex_lock(&mutex[i]);

                if (status == EBUSY)
                {
                    backoffs++;
                    printf("backward locker backing of at %d\n", i);
                    for (; i < 3; i++)
                    {
                        status = pthread_mutex_unlock(&mutex[i]);
                        if (status != 0)
                            err_abort(status, "Backoff");
                    }
                }
                else
                {
                    if (status != 0)
                        err_abort(status, "Lock mutex");
                    printf("backward locker got %d\n", i);
                }
            }

            if (yield_flag)
            {
                if (yield_flag > 0)
                    sched_yield();
                else
                    sleep(1);
            }
        }

        printf("lock backward got all locks, %d backoffs\n", backoffs);
        pthread_mutex_unlock(&mutex[2]);
        pthread_mutex_unlock(&mutex[1]);
        pthread_mutex_unlock(&mutex[0]);
        sched_yield();
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t forward, backward;
    int status;

    if (argc > 1)
        backoff = atoi(argv[1]);

    if (argc > 2)
        yield_flag = atoi(argv[2]);

    status = pthread_create(&forward, NULL, lock_forward, NULL);
    if (status != 0)
        err_abort(status, "Create forward");

    status = pthread_create(&backward, NULL, lock_backward, NULL);
    if (status != 0)
        err_abort(status, "Create backward");

    pthread_exit(NULL);
}
```

如果没有特殊防范机制，这个程序很快就会死锁，如果上面程序运行 `backoff 0`，就会看到死锁现象：

```shell
$ ./bin/backoff 0
backward lock got 2
backward locker got 1
forward lock got 0
```

上面两个线程都调用 `pthread_mutex_lock` 来加锁每个互斥量，由于线程从不同的端开始，所以它们在中间遇到时就会死锁。
而使用回退算法的程序，不管运行多少次循环，上面的程序都会正常执行，而不会发生死锁现象。

## 条件变量

条件变量是用来通知共享数据状态信息的。可以使用条件变量来通知队列已空、或队列非空、或任何其他需要由线程处理的共享数据状态。

当一个线程互斥地访问其享状态时，它可能发现在其他线程改变状态之前它什么也做不了。即没有破坏不变量，但是线程就是对当前状态不感兴趣。例如，一个处理队列的线程发现队列为空时，它只能等恃，直到有一个节点被添加进队列中。

条件变置不提供互斥，需要一个互斥量来同步对共享数据的访问。

一个条件变量应该与一个谓词相关，如果试图将一个条件变量与多个谓词相关，或者将多个条件变量与一个谓词相关，就有陷入死锁或者竞争问题的危险。

### 创建和释放条件变量

程序中由 `pthread_cond_t` 类型的变量来表示条件变量。如果声明了一个使用默认属性值的静态条件变量，则需要要使用 `PTHREAD_COND_TNTTIALIZER` 宏初始化，这样初始化的条件变量不必主动释放。

```c
pthread_cond_tcond = PTHREAD_COND_INITIALIZER;
```

下面时一个静态初始化条件变量的实例：

```c
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
```

有时无法静态地初始化一个条件变量，例如，当使用 `malloc` 分配一个包含条件变量的结构时，这时，你需要调用 `pthread_cond_init` 来动态地初始化条件变量。当动态初始化条件变量时，应该在不需要它时调用 `pthread_cond_destory` 来释放它。

```c
int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *condattr);
int pthread_cond_destroy(pthread_cond_t *cond);
```

下面是一个动态初始化条件变量的实例:

```c
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
```

### 等待条件变量和唤醒等待线程

每个条件变量必须与一个特定的互斥量、一个谓词条件相关联。当线程等待条件变量时，它必须轉相关互斥量锁住。记住，在阻寒线程之前，条件变量等待操作将解锁互斥量；而在重新返回线程之前，会再次锁住互斥量。

所有并发地（同时）等待同一个条件变量的线程心须指定同一个相关互斥量。例如，Pthreads不允许线程1使用互斥量 A 等待条件变量 A，而线程2使用互斥量 B 等待条件变量 A。不过，以下情况是十分合理的：线程1使用互斥量 A 等待条件变量 A，而线程2使用互斥量 A 等待条件变量 B。即，任何条件变量在特定时刻只能与一个互斥量相关联，而互斥量则可以同时与多个条件变过关联。

```c
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond *cond, pthread_mutex_t *mutex, struct timespec *expiration);
```

一旦有线程为某个谓词在等待一个条件变量，你可能需要唤醒它。Pthreads 提供了两种方式唤醒等待的线程：一个是“发信号”，一个是“广播”。发信号只唤醒一个等待该条件变量的线程，而广播将唤醒所有等待该条件变量的线程。

广播与发信号真正的区别是效率：广播将唤醒额外的等待线程，而这些线程会检测自己的谓词然后继续等待，通常，不能用发信号代替广播。“当有什么疑惑的时候，就使用广播”。

```c
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
```

下面实例展示了如何等待条件变量，唤醒正在睡眠的等待线程。
线程 `wait_thread` 等待指定时间后，设置 `value` 值后，发送信号给条件变量。
主线程调用 `pthread_cond_timedwait` 函数等待最多2秒，如果 `hibernation` 大于2秒则条件变量等待将会超时，返回 `ETIMEOUT`;
如果 `hibernation` 设置为2秒，则主线程与 `wait_thread` 线程发生竞争，每次运行结果可能不同；
如果 `hibertnation` 设置少于2秒，则条件变量等待永远不会超时。

```c
#include <pthread.h>
#include <time.h>
#include "errors.h"

typedef struct my_struct_tag
{
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             value;
} my_struct_t;

my_struct_t data = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};

int hibernation = 1;

void *wait_thread(void *arg)
{
    int status;

    sleep(hibernation);

    status = pthread_mutex_lock(&data.mutex);
    if (status != 0)
        err_abort(status, "Lock mutex");

    data.value = 1;

    status = pthread_cond_signal(&data.cond);
    if (status != 0)
        err_abort(status, "Signal condition");

    status = pthread_mutex_unlock(&data.mutex);
    if (status != 0)
        err_abort(status, "Unlock mutex");

    return NULL;
}

int main(int argc, char *argv[])
{
    int status;
    pthread_t wait_thread_id;
    struct timespec timeout;

    if (argc > 1)
        hibernation = atoi(argv[1]);

    status = pthread_create(&wait_thread_id, NULL, wait_thread, NULL);
    if (status != 0)
        err_abort(status, "Create wait thread");

    timeout.tv_sec = time(NULL) + 2;
    timeout.tv_nsec = 0;

    status = pthread_mutex_lock(&data.mutex);
    if (status != 0)
        err_abort(status, "Lock mutex");

    while (data.value == 0) {
        status = pthread_cond_timedwait(&data.cond, &data.mutex, &timeout);
        if (status == ETIMEDOUT) {
            printf("Condition wait time out.\n");
            break;
        } else if (status != 0)
            err_abort(status, "Wait on condition");
    }

    status = pthread_mutex_unlock(&data.mutex);
    if (status != 0)
        err_abort(status, "Unlock mutex");

    return 0;
}
```

### 闹钟实例最终版本

之前采用 `mutex` 实现的闹钟版本并不完美，它必须在处理完当前闹铃后，才能检测其他闹铃请求是否已经被加入了列表，即使新的请求
的到期时间比当前请求早。例如， 首先输入命令行 `10 message1`， 然后输入 `5 message2`，那么程序是无法预知后面5秒的闹钟加入到列表中来了，只能先处理完10秒的闹钟，才能继续处理后面的内容。

我们可以增加条件变量的使用来解决这个问题，新的版本使用一个超时条件变量操作代替睡眠操作，以等待闹钟到时。
当主线程在列表中添加了一个新的请求时，将发信号给条件变量，立刻唤醒 `alarm_thread` 线程。`alarm_thread` 线程可以重排等待的闹铃请求，然后重新等待。

你可以在 `alarm_cond.c` 获取源代码实现。