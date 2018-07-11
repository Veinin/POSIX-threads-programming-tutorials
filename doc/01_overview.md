# 概述

## 术语定义

### 同步与异步

同步（synchronous）意味着同时在一起工作。例如聊天室和在线会议就是同步的好例子，在聊天室中，人们对彼此的对话会立即得到反应。
同步相对来说比较简单，但开销相对较大。

异步（asynchronous) 表明事情相互独立地发生， 异步双方不需要共同的时钟，也就是接收方不知道发送方什么时候发送。例如论坛和电子邮件就是采用异步通信的一个好例子，这样沟通的双方都会有足够的时间去思考。
异步增加了复杂性以及更加麻烦的调试过程。如果你没有同时执行多个活动， 那么异步就没有什么优势。 如果你开始了一个异步活动， 然后什么也不做等待它结束， 则你并没有从异步那儿获得太多好处。

### 并发与并行

并发（concurrency ) 的意思是指事情同时发生。 也是指让实际上可能串行发生的事情好像同时发生一样。 并发描述了单处理器系统中线程或进程的行为特点。在 POSIX 中，并发的定义要求“延迟调用线程的函数不应
该导致其他线程的无限期延迟。

并行（parallelism) 指并发序列同时执行， 换言之，软件中的“并行”语言中的“并发”是相同的意思， 而区別于软件中的“并发”。指事情在相同的方向上独立进行（没有交错）。

真正的并行只能在多处理器系统中存在， 但是并发可以在单处理器系统和多处理器系统中都存在。
并发能够在单处理器系统中存在是因为并发实际上是并行的假象。 并行要求程序能够同时执行多个操作，而并发只要求程序能够假装同时执行多个操作。

### 单处理器和多处理器

单处理器是指一台计算机只有一个编程人员可见的执行单元（处理器）。对于拥有超标量体系结构、向量或者其他数学或 I/O 协处理器的单一通用处理器，我们仍然把它当成单处理器。

多处理器是指一台计算机拥有多个处理器，它们共享同一个指令集和相同的物理内存。虽然处理器不必同等地访问所有物理内存，但是每一个应该都能访问大部分内存。

### 线程安全和可重入

线程安全是指代码能够被多个线程调用而不会产生灾难性的结果。它补要求代码在多个线程中高效的运行，只要求能够安全的运行。人部分现行函数可以利用 Pthreads 提供的互斥量、 条件变量和线程私有数据来实现线程的安全。比如，在进入函数时加锁，在退出函数时解锁。这样的函数可以被多个线程调用，但一次只能有一个线程调用它。

“可重入”有时用来表示”有效的线程安全”。意味着函数不在连续的调用中保存静态数据，也不返回指向静态数据的指针。所有的数据都是由函数的调用程序提供的。重入函数不得调用非重入函数。

## 线程的好处

多线程编程模型具有以下优点：

- 在多处理器系统中开发程序的并行性，除了并行性这一优点是需要特殊硬件支持外， 其他优点对硬件不做要求。
- 在等待慢速外设 I/O 操作结束的同时， 程序可以执行其他计算， 为程序的并发提供更有效、 更自然的开发方式。
- 一种模块化编程模型， 能清晰地表达程序中独立事件间的相互关系。

## 线程的代价

任何事情都有代价，线程也不例外。在很多情形下好处超过了代价，在其他情形下则相反。

- 计算负荷。比如线程间同步会直接影响运行时间，对于两个总是同时使用的变量分别加以保护，这意味着你在同步上花费太多的时间而损失了并发。
- 编程规则。尽管线程编程模型的基本思想简单，但是编写实际的代码不是件容易的事。编写能够在多个线程中良好工作的代码需要认真的思考和计划。你需要明白同步协议和程序中的不变量（invariant), 你不得不避免死锁、竞争和优先级倒置。
- 更难以调试。调试不可避免地要改变事件的时序。在调试串行代码时不会有什么大问题，但是在调试异步代码时却是致命的。如果一个线程因调试陷阱而运行得稍微慢了，则你要跟踪的问题就可能不会出现。每个程序员都会遇到此类在调试时无法再现的错误，这在线程编程中会更加普遍。

## 选择线程还是不用线程

线程并非总是容易使用，而且并非总是可达到最好的性能。一些问题本身就是非并发的，添加线程线程只能降低程序的性能并使程序复杂。如果程序中的每一步都需要上一步的结果，则使用线程不会有任何帮助。每个线程不得不等待其他线程的结束。

最适合使用线程的应用包括以下这些：

- 计算密集型应用，为了能在多处理器系统上运行，将这些计算分解到多个线程中实现。
- I/O 密集型应用，为提高性能，将 I/O 操作重叠。很多线程可以同时等待不同的 I/O 操作。分布式服务器应用就是很好的实例，它们必须响应多个客户的请求，必须为通过慢速网络的连接主动提供 I/O 准备。

## POSIX线程概念

POSIX 线程线程 API 遵循国际正式标准 POSIX 1003.1c-1995, 我们将使用非正式的术语 "Pthreads" 代表 "POSIX 1003.1c-1995"。

线程系统包含三个基本要素：

- 执行环境，是并发实体的状态。 并发系统必须提供建立、 删除执行环境和独立维护它们状态的方式。
- 调度，决定在某个给定时刻该执行哪个环境（或环境组）， 并在不同的环境中切换。
- 同步，为并发执行的环境提供了协调访问共享资源的一种机制。

下表列出了上述三方面的几个不同的实例：

| 环境 | 执行环境 | 调度 | 同步 |
|---|---|---|---|
| 交通 |汽车 | 红绿灯 | 转变信号和刹车灯
| UNIX ( 无线程 ) | 进程 | 优先级| 等待和管道 |
| Pthreads | 线程 | 策略、 优先级 | 条件变量和互斥量 |

### 结构概述

使用 Pthreads, 通过调用 `pthread_create` 来创建执行环境（线程）。 创建一个线程同样也调度了该线程的执行，这将通过调用指定的 “**线程启动**” 函数开始。Pthreads 允许在创建线程时指定调度参数，或者在线程运行时设定。
当线程调用 `pthread_exit` 时退出，或者也可以从线程启动函数中返回时退出。

基本的 Pthreads 同步模型使用 **互斥量** 来保护共享数据，使用 **条件变量** 来通信，还可以使用其他的同步机制，如 **信号量**、**管道** 和 **消息队列**。
互斥量允许线程在访问共享数据时锁定它，以避免其他线程的干扰。条件变量允许线程等待共享数据到达某个期望的状态（例如队列非空或者资源可用）。

### 类型和接口

POSIX 线程数据类型：

| 类型 | 描述 |
|---|---|
| pthread_t           | 线程标识符 |
| pthreae_mutex_t     | 互斥量 |
| pthread_code_t      | 条件变量 |
| pthread_key_t       | 线程私有权握访问键 |
| pthread_attr_t      | 线程属性对象 |
| pthread_mutexattr_t | 互斥量属性对 |
| pthread_condattr_t  | 条件变属性对象 |
| pthread_once_t      | “一次性初始化”控制变量 |

### 错误检查

在传统的UNIX系统和原来的标准中．errno 是一个外部整型变量。由于该变量一次只能有一个值，所以只能支持进程中的单一执行流程。
传统的报错机制有许多问题，包括很难创建在报错的同时返回一个有用的 -1 值的函数。当引入多线程时会有更严重的问题。
Pthreads 修订版是 POSIX 中第一个与传统的 UNIX 和 C 语言报错机制相分离的部分。
Pthreads 中的新函数通过返回值来表示错误状态，而不是用变量。当成功时，Pthreads 函数返回 0, 并包含一个额外的输出参数来指向存有“有用结果”
的地址。当发生错误时，函数返回一个包含在 `errno` 变量以支持其他使用 `<errao.h>` 文件中的错误代码。

下面程序是一个典型的线程错误检查代码，因为 `pthread_t` 变量拥有一个无效的值，所以在使用 `pthread_join` 在遇到无效线程ID时会返回错误代码 `ESRCH`。
运行下面程序将显示错误消息：`error 3: No such process`。

```c
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main()
{
    pthread_t thread;
    int status;

    status = pthread_join(thread, NULL);
    if (status != 0)
        fprintf(stderr, "error %d: %s\n", status, strerror(status));

    return status;
}
```

为了避免在实例代码的每个函数调用中都增加报错和退出的代码段，我们需要写两个报错宏使用 err_abort 检测标准的 Pthreads 错误， 使用 errno_abort 检测传统的 errno 错误变量方式。

```c
#define err_abort(code, text) \
    do {\
        fprintf(stderr, "%s at \"%s\":%d: %s\n",\
            text, __FILE__, __LINE__, strerror(code));\
        abort();\
    } while(0)

#define errno_abort(text) \
    do {\
        fprintf(stderr, "%s at \"%s\":%d: %s\n",\
            text, __FILE__, __LINE__, strerror(errno));\
        abort();\
    } while(0)
```

## 异步编程举例

下面使用一个简单的闹钟实例程序来演示基本的异歩编程方法。该程序循环接受用户输入信息，直到出错或者输入完毕，用户输入的每行信息中，第一部分是闹钟等待的时间（ 以秒为单位），第二部分是闹钟时间到迖时显示的文本消息。

### 同步版本

一直同步等待 `fgets` 产生输入，然后根据输入的秒数进行等待指定时间，最后输出闹钟响起的消息。该程序的问题是一次只能处理一个闹钟请求，如果你的程序设置了一个10分钟闹钟，就不能再继续让它在5分钟时响起另外一个闹钟。

```c
#include "errors.h"

int main(void) {
    int seconds;
    char line[128];
    char message[64];

    while(1) {
        printf("Alarm> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            exit(0);

        if (strlen(line) == 0)
            continue;

        if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
            fprintf(stderr, "Bad command\n");
        } else {
            sleep(seconds);
            printf("(%d) %s\n", seconds, message);
        }
    }
}
```

### 多进程版本

为每个命令使用 `fork` 调用生成一个独立的子进程来处理闹钟。`fork` 版本是异步方式的的一种实现，该程序可以随时输入命令行，它们被彼此独立地执行。 新版本并不比同步版本复杂多少。
该版本的主要难点在于对所有己终止子进程的 `reap`。如果程序不做这个工作，则要等到程序退出的时候由系统回收，通常回收子进程的方法是调用某个 `wait` 系列函数。在本例中，我们调用 waitpid 函数，并设置 WNOHANG（父进程不必挂等待子进程的结束）。

```c
#include <sys/types.h>
#include <wait.h>
#include "errors.h"

int main(void) {
    pid_t pid;
    int seconds;
    char line[128];
    char message[64];

    while(1) {
        printf("Alarm> ");
        if (fgets(line, sizeof(line), stdin) == NULL)
            exit(0);

        if (strlen(line) <= 1)
            continue;

        if (sscanf(line, "%d %64[^\n]", &seconds, message) < 2) {
            fprintf(stderr, "Bad command\n");
        } else {
            pid = fork();
            if (pid == -1)
                errno_abort("Fork");

            if (pid == (pid_t)0) {
                sleep(seconds);
                printf("(%d) %s\n", seconds, message);
                exit(0);
            } else {
                do {
                    pid = waitpid((pid_t)-1, NULL, WNOHANG);
                    if (pid == (pid_t) -1)
                        errno_abort("Wait for child");
                } while(pid != (pid_t)0);
            }
        }
    }
}
```

### 多线程版本

多线程版本与多进程十分相似，只是使用线程而非子进程来实现异步闹钟。本例中用到了以下三个Pthread函数：

- `pthread_create` 函数建立一个线程， 运行由第三个参数 `alarm_thread` 指定的例程，并返回线程标识符 ID (保存在 `thread` 引用的变量中）
- `pthread_self` 获取当前线程标识符 ID。
- `pthread_detach` 函数允许在当线程终止时立刻回收线程资源。

```c
#include "errors.h"
#include <pthread.h>

typedef struct alarm_tag {
    int  seconds;
    char message[64];
} alarm_t;

void *alarm_thread(void *arg) {
    alarm_t *alarm = (alarm_t*)arg;
    int status;

    status = pthread_detach(pthread_self());
    if (status != 0)
        err_abort(status, "Detach thread");

    sleep(alarm->seconds);
    printf("(%d) %s\n", alarm->seconds, alarm->message);
    free(alarm);
    return NULL;
}

int main(void) {
    int status;
    int seconds;
    char line[128];
    alarm_t *alarm;
    pthread_t thread;

    while(1) {
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
            status = pthread_create(&thread, NULL, alarm_thread, alarm);
            if (status != 0)
                err_abort(status, "Create alarm thread");
        }
    }
}
```