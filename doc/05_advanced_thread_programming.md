# 线程高级编程

## 一次性初始化

一些事情仅仅需要做一次，不管是什么。在主函数中并且在调用任何其他依赖于初始化的事物之前，这时初始化应用最容易，特别是在创造任何线程之前初始化它需要的数据，如互斥量、线程特定数据键等。

在传统的顺序编程中，一次性初始化经常通过使用布尔变量来管理。控制变量被静态地初始化为 0,而任何依赖于初始化的代码都能测试该变量。如果变量值仍然为 0, 则它能实行初始化，然后将变量置为 1，以后检查的代码将跳过初始化。如下面代码示例：

```c
bool initialized = false;

void init()
{
    if (initialized)
        return;

    // TODO
    initialized = true
}
```

但在使用多线程时，上述操作就不是那么容易了。如果多个线程并发地执行初始化序列代码， 2 个线程可能都发现 `initialized` 为0，并且都执行初始化，而且该过程本该仅仅执行一次，那么上面代码就会立马发生不可预估的错误。

对于多线程环境下初始化的状态有两种方式：

- 使用一个静态初始化的互斥量来编写一次性初始化代码。
- 无法静态初始化一个互斥量时，使用 `pthread_once`。

对于 `pthread_once` 初始化，需要声明类型为 `pthread_once_t` 的一个控制变量，且该控制变量必须使用 `PTHREAD_ONCE_INIT` 宏进行静态初始化。`pthread_once` 首先检查控制变量，以判断是否已经完成初始化，如果完成，则什么都不做并立刻返回；否则，`pthread_once` 会调用初始化函数，并且记录初始化完成。如果在一个线程初始化时，另外一个线程也调用了`pthread_once`，则调用线程会阻塞等待，直到正在初始化的线程返回，这样就确保了所以状态一定会正确初始化完成。

下面是一个使用 `pthread_once` 来初始化的实例：

```c
#include <pthread.h>
#include "errors.h"

pthread_once_t once_block = PTHREAD_ONCE_INIT;
pthread_mutex_t mutex;

void once_init_routine(void)
{
    int status;

    status = pthread_mutex_init(&mutex, NULL);
    if (status != 0)
        err_abort(status, "Init mutex");
}

void *thread_routine(void *arg)
{
    int status;

    status = pthread_once(&once_block, once_init_routine);
    if (status != 0)
        err_abort(status, "Once init");

    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        err_abort(status, "Init mutex");

    printf("thread_toutine has locked the mutex.\n");

    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        err_abort(status, "Destroy mutex");

    return NULL;
}

int main()
{
    int status;
    pthread_t thread_id;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    status = pthread_once(&once_block, once_init_routine);
    if (status != 0)
        err_abort(status, "Once init");

    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        err_abort(status, "Init mutex");

    printf("Main has locked the mutex.\n");

    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        err_abort(status, "Destroy mutex");

    status = pthread_join(thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join thread");

    return 0;
}
```

上面程序，唯一的临界共享数据实际是 `once_block`，主线程和线程 `thread_routine` 都会调用 `pthread_once` 进行初始化，但只会有一个线程会执行初始化函数。

## 属性

当我们创建线程或动态初始化互斥量和条件变量时，通常使用空指针作为第二个参数，这个参数实际上是指向一个属性对象的指针。空指针表明，Pthreads 应该为所有属性假定默认值，就像静态初始化互斥量或条件变量时一样。

一个属性对象是当初始化一个对象时提供的一个扩展参数表，可以提供更加高级的功能。类型 `pthread_attr_t` 代表一个属性对象，线程、 互斥置和条件变量都有自己特殊的属性对象类型， 分别是 `pthread_attr_t`、 `pthread_mutexattr_t` 和 `pthread_condattr_t`。

### 互斥量属性

Pthreads 为互斥量创建定义下列属性：`pshared`、`pratocol` 和 `prioceiling`。通过调用 `pthread_mutexattr_init` 初始化互斥量属性，指定一个指向类型 `pthread_mutexattr_t` 变量的指针。

```c
pthread_mutexattr_t mutex_attr;
int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);
int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared)
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared);
```

下面程序显示了如何设置属性对象来创建使用 `pshared` 属性的互斥量，并且获取 `pshared` 值，打印输出 1：

```c
#include <pthread.h>
#include "errors.h"

pthread_mutex_t mutex;

int main()
{
    int status;
    int pshared;
    pthread_mutexattr_t mutex_attr;

    status = pthread_mutexattr_init(&mutex_attr);
    if (status != 0)
        err_abort(status, "Init mutex attr");

    status = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    if (status != 0)
        err_abort(status, "Set pshared");

    status = pthread_mutex_init(&mutex, &mutex_attr);
    if (status != 0)
        err_abort(status, "Init mutex");

    status = pthread_mutexattr_getpshared(&mutex_attr, &pshared);
    if (status != 0)
        err_abort(status, "Get pshared");

    printf("pshared: %d\n", pshared);

    return 0;
}
```

### 条件变量属性

Pthreads 为条件变量的创建仅定义了一个属性 `pshared`。使用 `pthread_condattr_init` 初始化条件变量属性对象，设置一个指向类型 `pthread_condattr_t` 变量的指针。可以通过调用 `pthread_condattr_setpshared` 设置 `pshared`。

该属性默认值时 `PTHREAD_PROCESS_PRIVATE`，如果条件变量属性需要被多个线程使用，可以设置值为 `PTHREAD_PROCESS_SHAREAD`。

下面程序演示了如何使用条件变量的 `pshared` 属性来创建设置一个条件变量:

```c
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
```

### 线程属性

POSIX 为线程创建定义下列属性：detachstate、stacksize、stackaddr、scope、inheritsched、schedpolicy 和 schedparam。并不是所有系统都支持以上所有的属性，因此需要在使用前检查系统文档。

所有的 Pthreads 系统都支持 `detachstate` 属性 ，该属性的值可以是 `PTHREAD_CREATE_JOINABLE` 或 `PTHREAD_CREATE_DETACHED`。
默认的线程被创建为可连接的(joinable)，即意味着由 `pthread_create` 创建的该线程ID 能被用来与线程连接并获得它的返回值，或取消它。
如果设置为 `PTHREAD_CREATE_DETACHED`，则该属性对象创建的线程 ID 不能被使用，线程终止时，线程的所有资源都会被系统立刻回收。所以，在创建已经知道不需要取消或连接的线程时，应该以可分离的方式创建它们。

```c
pthread_attr_t attr;
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int *detachstate);
```

如果系统定义了标志 `_POSIX_THREAD_ATTR_STACKSIZE`，就可以设置 `stacksize` 属性，指定使用属性对象创建的线程栈的最小值。但栈大小不是可移植的，你应该小心使用它。
如果系统定义了标志 `_POSIX_THREAD_ATTR_STACKADDR`，就可以设置 `stackaddr` 属性，为指定线程指定一个存储器区域来作为堆栈使用。

下面程序演示了实际中的某些属性对象的使用实例：

```c
#include <limits.h>
#include <pthread.h>
#include "errors.h"

void *thread_routine(void *arg)
{
    printf("The thread is here\n");
    return NULL;
}

int main()
{
    pthread_t thread_id;
    pthread_attr_t thread_attr;
    struct sched_param thread_param;
    size_t stack_size;
    int status;

    status = pthread_attr_init(&thread_attr);
    if (status != 0)
        err_abort(status, "Create attr");

    status = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (status != 0)
        err_abort(status, "Set detach");

#ifdef _POSIX_THREAD_ATTR_STACKSIZE
    status = pthread_attr_getstacksize(&thread_attr, &stack_size);
    if (status != 0)
        err_abort(status, "Get stack size");

    printf("Default stack size is %d; minimum is %d\n", stack_size, PTHREAD_STACK_MIN);

    status = pthread_attr_setstacksize(&thread_attr, PTHREAD_STACK_MIN*2);
    if (status != 0)
        err_abort(status, "Set stack size");
#endif

    status = pthread_create(&thread_id, &thread_attr, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    printf("Main exiting\n");
    pthread_exit(NULL);
    return 0;
}
```

## 取消

大部分时间每个线程独立地运行着，完成一个特定的工作，并且自己退出。但是有时一个线程被创建并不需要一定完成某件事情。例如用户可以单击按钮取消停止长时间的搜索操作。取消一个线程就像告诉一个人停止他正在做的工作一样。Pthreads 允许每个线程控制自己的结束，它能恢复程序不变量并解锁互斥量。当线程完成一些重要的操作时它甚至能推迟取消。

以下是常用的线程取消函数：

```c
int pthread_cancel(pthread_t thread);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);
void pthread_testcancel(void);
void pthread_cleanup_push(void (*routine)(void *), void *arg);
void pthread_cleanup_pop(int execute);
```

Pthread 支持三种取消模式，模式为两位二进制编码，称为“取消状态”和“取消类型”。每种模式实质上包括开、关两种状态。取消状态可以是“启用”（ enable)或“禁用”（disable)，取消类型可以是被“推迟” 或 “异步。如果取消状态被禁用，那么其他取消模式都会失效，相反则可以执行“推迟”或“异步”模式。

| 模式 | 状态 | 类型 | 含义 |
|---|---|---|---|
| Off（关）| 禁用 | 二者之一 | 取消被推迟，直到启动取消模式 |
| Deferred（推迟） | 启用 | 推迟 | 在下一个取消点执行取消 |
| Asynchronous（异步） | 启用 | 异步 | 可以随时执行取消 |

为取消一个线程， 你需要线程的标识符 ID， 即由 `pthread_create` 返回给创建者或由 `pthread_self` 返回给线程自己的 `pthread_t` 值。如果没有一个线程的标识符 TD， 就不能取消线程。

取消一个线程是异步的， 当 `pthread_cancel` 调用返回时， 线程未必已经被取消，可能仅仅被通知有一个针对它的未解决的取消请求。如果需要知道线程在何时实际终止，就必须在取消它之后调用 `pthread_join` 与它连接。

也有被称为 `pthread_testcancel` 的特殊函数， 该函数仅仅是一个推迟的取消点。如果线程没被要求终止，它将很快返回，这允许你将任何函数转变为取消点。

下面是一个在循环内调用 `pthread_testcancel` 来对一个延迟取消反应的线程实例：

```c
#include <pthread.h>
#include "errors.h"

static int counter;

void *thread_routine(void *arg)
{
    printf("thread_routine starting\n");
    for (counter == 0; ; counter++)
    {
        if ((counter % 1000) == 0)
        {
            printf("calling testcancel\n");
            pthread_testcancel();
        }
    }
}

int main()
{
    pthread_t thread_id;
    void *result;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    sleep(2);

    printf("callling cancel\n");

    status = pthread_cancel(thread_id);
    if (status != 0)
        err_abort(status, "Cancel thread");

    printf("calling join\n");

    status = pthread_join(thread_id, &result);
    if (status != 0)
        err_abort(status, "Join thread");

    if (result == PTHREAD_CANCELED)
        printf("Thread canceled at iteration %d\n", counter);
    else
        printf("Thread was not canceled\n");

    return 0;
}
```

### 推迟取消

“推迟取消”意味着线程的取消类型被设置为 `PTHREAD_DEFERRED`，线程的取消使能属性被设置为 `PTHREAD_CANCEL_ENABLE`，线程将仅仅在到达取消点时才响应取消请求。

大多数取消点包含可以“无限”时间阻塞线程的 I/O 操作，它们是可取消的，以便等待能被打断，比如 `wait`、`read` 这样的函数函数。
你可以在下面这个链接中查找到所有可能的取消点：
[Pthreads-Cancellation-points](http://man7.org/linux/man-pages/man7/pthreads.7.html)

如果需要保证取消不能在一个特别的取消点或取消点的一些顺序期间发生，可以暂时在代码的那个区域停用取消。下面程序是一个实例：

```c
#include <pthread.h>
#include "errors.h"

static int counter;

void *thread_routine(void *arg)
{
    int state;
    int status;

    printf("thread_routine starting\n");
    for (counter == 0;; counter++)
    {
        if ((counter % 755) == 0)
        {
            status = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &state);
            if (status != 0)
                err_abort(status, "Disable cancel");

            sleep(1);

            status = pthread_setcancelstate(state, &state);
            if (status != 0)
                err_abort(status, "Restore cancel");
        }
        else if ((counter % 1000) == 0)
        {
            printf("calling testcancel\n");
            pthread_testcancel();
        }
    }
}

int main()
{
    pthread_t thread_id;
    void *result;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    sleep(2);

    printf("callling cancel\n");

    status = pthread_cancel(thread_id);
    if (status != 0)
        err_abort(status, "Cancel thread");

    printf("calling join\n");

    status = pthread_join(thread_id, &result);
    if (status != 0)
        err_abort(status, "Join thread");

    if (result == PTHREAD_CANCELED)
        printf("Thread canceled at iteration %d\n", counter);
    else
        printf("Thread was not canceled\n");

    return 0;
}
```

### 异步取消

如果目标线程不需要使用取消点来査询取消请求。对于运行一个紧密计算循环的线程（例如，在找一个素数因素）而言是非常珍贵的，因为那种情况下调用 `pthread_testcancel` 的开销在可能是严重的。

异步取消线程很难确保目标线程安全的执行取消，例如当你调用 `malloc` 时，系统为你分配一些堆内存，但 `malloc` 可能在很多地方被异步取消打断，可能在分配内存前，或可能在分配内存后、也可能在保存地址返回前被打断。无论哪种情况，你的代码保存的内存地址变量将是未初始化的，这就很可能造成内存泄漏。

所以，在你的任何代码里面应该 **避免异步的取消** ！我们很难正确使用异步取消，并且很少有用。

除非当函数被记录为“异步取消安全”的，否则当异步取消被启用时你不该调用任何函数。Pthreads 建议所有的库函数应该记录它们是否是异步取消安全的。如果函数的描述没有具体的说明，则你应该总是假定它不是异步取消安全的。

下面是一个计算密集的循环中异步取消的使用实例，但是如果在循环内有任何函数调用，程序将变得不可靠，而推迟取消的版本将能继续正确工作：

```c
#include <pthread.h>
#include "errors.h"

#define SIZE 10

static int matrixa[SIZE][SIZE];
static int matrixb[SIZE][SIZE];
static int matrixc[SIZE][SIZE];

void *thread_routine(void *arg)
{
    int cancel_type, status;
    int i, j, k, value = 1;

    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
        {
            matrixa[i][j] = i;
            matrixb[i][j] = j;
        }

    while (1) {
        status = pthread_setcancelstate(PTHREAD_CANCEL_ASYNCHRONOUS, &cancel_type);
        if (status != 0)
            err_abort(status, "Set cancel type");

        for (i = 0; i < SIZE; i++)
        {
            for (j = 0; j < SIZE; j++)
            {
                matrixc[i][j]= 0;
                for (k = 0; k < SIZE; k++)
                    matrixc[i][j] += matrixa[i][k] * matrixb[k][j];
            }
        }

        status = pthread_setcancelstate(cancel_type, &cancel_type);
        if (status != 0)
            err_abort(status, "Set cancel type");

        for (i = 0; i < SIZE; i++)
            for (j = 0; j < SIZE; j++)
                matrixa[i][j] = matrixc[i][j];
    }
}

int main()
{
    pthread_t thread_id;
    void *result;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create thread");

    sleep(1);

    printf("callling cancel\n");

    status = pthread_cancel(thread_id);
    if (status != 0)
        err_abort(status, "Cancel thread");

    printf("calling join\n");

    status = pthread_join(thread_id, &result);
    if (status != 0)
        err_abort(status, "Join thread");

    if (result == PTHREAD_CANCELED)
        printf("Thread canceled\n");
    else
        printf("Thread was not canceled\n");

    return 0;
}
```

### 清除

当一个代码段被取消时，需要恢复一些状态，必须使用清除处理器。例如当线程在等待一个条件变量时被取消，它将被唤醒，并保持互斥量加锁状态。在线程终止前，通常需要恢复不变量，且它总是需要释放互斥量。

可以把每个线程考虑为有一个活动的清除处理函数的栈。 调用 `pthread_cleanup_push` 将清除处理函数加到栈中， 调用 `pthread_cleanup_pop` 删除最近增加的处理函数。当线程被取消时或当它调用 `pthread_exit` 退出时，Pthreads 从最近增加的清除处理函数幵始，依次调用各个活动的清除处理函数，当所有活动的清除处理函数返回时，线程被终止。

下面程序演示了当一个条件变量等待被取消时，使用清除处理函数来释放互斥量：

```c
#include <pthread.h>
#include "errors.h"

#define THREADS 5

typedef struct control_tag
{
    int counter, bysy;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
} control_t;

control_t control = {0, 1, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER};

void cleanup_handler(void *arg)
{
    control_t *st = (control_t *)arg;
    int status;

    st->counter--;
    printf("cleaup_handler: counter == %d\n", st->counter);

    status = pthread_mutex_unlock(&st->mutex);
    if (status != 0)
        err_abort(status, "Unlock in cleanup handler");
}

void *thread_routine(void *arg)
{
    int status;

    pthread_cleanup_push(cleanup_handler, (void *)&control);

    status = pthread_mutex_lock(&control.mutex);
    if (status != 0)
        err_abort(status, "Mutex lock");

    control.counter++;

    while (control.bysy)
    {
        status = pthread_cond_wait(&control.cv, &control.mutex);
        if (status != 0)
            err_abort(status, "Wait on condition");
    }

    pthread_cleanup_pop(1);
    return NULL;
}

int main()
{
    pthread_t thread_id[THREADS];
    int count;
    void *result;
    int status;

    for (count = 0; count < THREADS; count++)
    {
        status = pthread_create(&thread_id[count], NULL, thread_routine, NULL);
        if (status != 0)
            err_abort(status, "Create thread");
    }

    sleep(2);

    for (count = 0; count < THREADS; count++)
    {
        status = pthread_cancel(thread_id[count]);
        if (status != 0)
            err_abort(status, "Cancel thread");

        status = pthread_join(thread_id[count], &result);
        if (status != 0)
            err_abort(status, "Join thread");

        if (result == PTHREAD_CANCELED)
            printf("Thread %d canceled\n", count);
        else
            printf("Thread %d was not canceled\n", count);
    }
}
```

如果你的一个线程创建了一套线程来“转包”一些功能（如并行算术运算），并且当分包线程在进行中时“承包线程”被取消，你可能不希望留着分包线程继续运行。相反，可以把取消操作“传递”到每个“分包线程”，让它们独立地处理自己的终止过程。当“承包线程”取消它们时，不应该连接分包线程来推迟取消，相反，可以取消每个线程并且使用 `pthread_detach` 很快地分离它。当它们完成时，分包线程的资源就能够很快被重用， 而“承包线程” 同时能独立地完成一些事情。

以下程序演示了同时独立取消“分包线程”的一个方法：

```c
#include <pthread.h>
#include "errors.h"

#define THREADS 5

typedef struct team_tag {
    int         join_i;
    pthread_t   workers[THREADS];
} team_t;

void *worker_routine(void *arg)
{
    int counter;

    for(counter = 0; ; counter++)
        if ((counter % 1000) == 0)
            pthread_testcancel();
}

void cleanup(void *arg)
{
    team_t *team = (team_t *)arg;
    int count, status;

    for (count = team->join_i; count < THREADS; count++) {
        status = pthread_cancel(team->workers[count]);
        if (status != 0)
            err_abort(status, "Cancel worker");

        status = pthread_detach(team->workers[count]);
        if (status != 0)
            err_abort(status, "Detach worker");

        printf("Cleanup: canceled %d\n", count);
    }
}

void *thread_routine(void *arg)
{
    team_t team;
    int count;
    void *result;
    int status;

    for (count = 0; count < THREADS; count++) {
        status = pthread_create(&team.workers[count], NULL, worker_routine, NULL);
        if (status != 0)
            err_abort(status, "Create worker");
    }

    pthread_cleanup_push(cleanup, (void *)&team);

    for (team.join_i = 0; team.join_i < THREADS; team.join_i++) {
        status = pthread_join(team.workers[team.join_i], &result);
        if (status != 0)
            err_abort(status, "Join worker");
    }

    pthread_cleanup_pop(1);
    return NULL;
}

int main()
{
    pthread_t thread_id;
    int status;

    status = pthread_create(&thread_id, NULL, thread_routine, NULL);
    if (status != 0)
        err_abort(status, "Create team");

    sleep(5);

    status = pthread_cancel(thread_id);
    if (status != 0)
        err_abort(status, "Cancel team");

    status = pthread_join(thread_id, NULL);
    if (status != 0)
        err_abort(status, "Join team");

    return 0;
}
```

## 线程私有数据

在进程内的所有线程共享相同的地址空间，即意味着任何声明为静态或外部的变量，或在进程堆声明的变量，都可以被进
程内所有的线程读写。

当线程需要一个私有变量时，必须首先决定所有的线程是否共享相同的值，或者线程是否应该有它自己的值。如果它们共享变量，则可以使用静态或外部数据，就像你能在一个单线程程序做的那样；然而，必须同步跨越多线程对共享数据的存取， 运通常通过增加一个或多个互斤量来完成。

如果每个线程都需要一个私有变量值，则必须在某处存储所有的值。线程私有数据允许每个线程保有一份变量的拷贝，好像每个线程有一连串通过公共的“键”值索引的私有数据值。

### 建立和使用线程私有数据

线程私有数据键在程序中是由类型 `pthread_key_t` 来表示的。

在任何线程试图使用键以前，创建线程私有数据键最容易的方法是调用 `pthread_key_create`，但必须保证 `pthread_key_create` 对于每个 `pthread_key_t` 变童仅仅调用一次。如果将一个键创建两次，其实是在创建两个不同的键。第二个键将覆盖第一个，第一个键与任何线程为其设置的值一起将永远地丢失。所以，最容易一次性创建一个键的方法是使用 `pthread_once`。

当程序不再需要时，你可以调用 `pthread_key_delete` 释放一个线程私有数据键。

在私有数据创建后，你可以使用 `pthread_getspecific` 函数来获得线程当前的键值，或调用 `pthread_setspecific` 来改变当前的键值。

```c
pthread_key_t key;
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
void pthread_setspecific(pthread_key_t key, const void *value);
void *pthread_getspecific(pthread_key_t);
```

下面是一个建立和使用线程私有数据的实例：

```c
#include <pthread.h>
#include "errors.h"

typedef struct tsd_tag {
    pthread_t   thread_id;
    char        *string;
} tsd_t;

pthread_key_t tsd_key;
pthread_once_t key_one = PTHREAD_ONCE_INIT;

void once_routine(void)
{
    int status;

    printf("initializing key\n");
    status = pthread_key_create(&tsd_key, NULL);
    if (status != 0)
        err_abort(status, "Create key");
}

void *thread_routine(void *arg)
{
    tsd_t *value;
    int status;

    status = pthread_once(&key_one, once_routine);
    if (status != 0)
        err_abort(status, "Once init");

    value = (tsd_t*)malloc(sizeof(tsd_t));
    if (value == NULL)
        errno_abort("Allocate key value");

    status = pthread_setspecific(tsd_key, value);
    if (status != 0)
        err_abort(status, "Set tsd");

    printf("%s set tsd value %p\n", (char*)arg, value);

    value->thread_id = pthread_self();
    value->string = (char*)arg;

    value = (tsd_t*)pthread_getspecific(tsd_key);
    printf("%s starting...\n", value->string);

    sleep(2);

    value = (tsd_t*)pthread_getspecific(tsd_key);
    printf("%s done...\n", value->string);

    return NULL;
}

int main()
{
    int status;
    pthread_t thread1, thread2;

    status = pthread_create(&thread1, NULL, thread_routine, "thread 1");
    if (status != 0)
        err_abort(status, "Create thread 1");

    status = pthread_create(&thread2, NULL, thread_routine, "thread 2");
    if (status != 0)
        err_abort(status, "Create thread 2");

    pthread_exit(NULL);
}
```

### 使用 destructor 函数

当一个线程退出时，它有一些为线程私有数据键定义的值，通常需要处理它们。当你创建一个线程私有数据键时，Pthreads 允许你定义 `destructor` 函数。当具有非空的私有数据键值的一个线程终止时，键的 `destructor` (如果存在） 将以键的当前值为参数被调用。

下列程序表明了当一个线程终止时使用线程私有数据的 `destructors` 释放存储器。 它还跟踪有多少线程正在使用线程私有数据， 并且当最后线程的 `destructor` 被调用时， 删除线程私有数据键。

```c
#include <pthread.h>
#include "errors.h"

typedef struct private_tag {
    pthread_t   thread_id;
    char        *string;
} private_t;

pthread_key_t identity_key;
pthread_mutex_t identity_key_mutex = PTHREAD_MUTEX_INITIALIZER;
long identity_key_counter = 0;

void identity_key_destructor(void *value)
{
    private_t *private = (private_t*)value;
    int status;

    printf("thread \"%s\" exiting...\n", private->string);
    free(value);

    status = pthread_mutex_lock(&identity_key_mutex);
    if (status != 0)
        err_abort(status, "Lock key mutex");

    identity_key_counter--;
    if (identity_key_counter <= 0) {
        status = pthread_key_delete(identity_key);
        if (status != 0)
            err_abort(status, "Delete key");
        printf("key delete...\n");
    }

    status = pthread_mutex_unlock(&identity_key_mutex);
    if (status != 0)
            err_abort(status, "Unlock key mutex");
}

void *identity_key_get(void)
{
    void *value;
    int status;

    value = pthread_getspecific(identity_key);
    if (value == NULL) {
        value = malloc(sizeof(private_t));
        if (value == NULL)
            errno_abort("Allocate key value");

        status = pthread_setspecific(identity_key, value);
        if (status != 0)
            err_abort(status, "Set TSD");
    }

    return value;
}

void *thread_routine(void *arg)
{
    private_t *value;

    value = (private_t*) identity_key_get();
    value->thread_id = pthread_self();
    value->string = (char*)arg;
    printf("thread \"%s\" starting...\n", value->string);
    sleep(2);
    return NULL;
}

int main()
{
    pthread_t thread_1, thread_2;
    private_t *value;
    int status;

    status = pthread_key_create(&identity_key, identity_key_destructor);
    if (status != 0)
            err_abort(status, "Create key");

    identity_key_counter = 3;

    value = (private_t*)identity_key_get();
    value->thread_id = pthread_self();
    value->string = "Main thread";

    status = pthread_create(&thread_1, NULL, thread_routine, "thread 1");
    if (status != 0)
        err_abort(status, "Create thread 1");

    status = pthread_create(&thread_2, NULL, thread_routine, "thread 2");
    if (status != 0)
        err_abort(status, "Create thread 2");

    pthread_exit(NULL);
}
```

## 线程实时调度

“受限制”的响应时间不一定是“快”的反应，而是确实意味着“可预知”的响应速度。必须有一些方法来定义一个时间跨度，在该时间段内一系列操作保证被完成。例如控制一个核反应堆的系统比你将写的大多数程序有更严格的响应要求，并且没能满足反应堆要求的后果是更严重的。

很多代码将需要在“确定的反应时间”内提供一些“达到要求水平的服务”，我们称为实时编程。

实时编程分为“硬实时”和“软实时”。“硬实时”是不可原谅的，如燃料干的调整被推迟几微妙，你的核反应堆将会很危险；“软实时”意味着你大部分时间需要满足调度要求，但是如果不能能满足，后果也不是很严重。

### POSIX 实时选项

优先级调度允许程序员给系统提供了任何两个线程间相。无论何时当多个线程准备好执行时，系统将选择最高优先级的线程。

#### 调度策略和优先级。

调度策略允许设置各个调度策略的最小和最大优先级。POXIS 标准提供两种调度策略（`SCHED_FIFO` 和 `SCHED_RR`)。

- `SCHED_FIFO`(先入先出）策略允许一个线程运行直到有更高优先级的线程准备好，或者直到它自愿阻塞自己。
- `SCHED_RR`(轮循），和先入先出策略是基本相同的，不同之处在于：如果有一个 `SCHED_RR`策略的线程执行了超过一个固定的时期（时间片间隔）没有阻塞，而另外的`SCHED_RR` 或 `SCHED_FIFO` 策略的相同优先级的线程准备好时，运行的线程将被抢占以使准备好的线程可以执行。

程序 `sched_attr.c` 显示了如何使用属性对象来创建一个具有显式的调度策略和优先级的线程。
程序 `sched_thread.c` 显示了如何为一个正在运行的线程修改实时调度策略和参数。

#### 竞争范围和分配域。

如果你正在写一个实时的应用程序，应该知道系统对这些控制量设置的支持，否则它们可能没有什么关系。

- **竞争范围**，它描述了线程为处理器资源而竞争的方式。系统竞争范围意味着线程与进程之外的线程竞争处理器资源。一个进程内的髙优先级系统竞争范围线程能阻止其他进程内的系统竞争范围线程运行。进程竞争范围指线程仅仅在同一进程内相互竞争。可以使用 `pthread_attr_setscope` 设置竞争范围。
- **分配域**，分配域是系统内线程可以为其竞争的处理器的集合。一个系统叫以有一个以上的分配领域，每个包含一个以上的处理器。在一个单处理机系统内，分配域将只包含一个处理器，但是你仍然可以有多个分配域。在一台多处理机上，各个分配领域可以包含从一个处理器到系统中所有的处理器。
