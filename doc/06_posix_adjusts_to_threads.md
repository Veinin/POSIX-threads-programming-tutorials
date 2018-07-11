# POSIX 针对线程的调整

## 多线程与fork

多线程进程调用 `fork` 创造的子进程，只有调用 `fork` 的线程在子进程内存在，除了当前调用的线程，其他线程在子进程中都会消失，但消失的线程状态仍然保留为调用 `fork` 时的相同状态，线程会拥有与在父进程内当前调用线程的相同状态，拥有相同的互斥量，同样的线程私有数据键值等。

在 `forked` 的进程中只有一个线程，消失的其他线程并不会调用诸如 `pthread_exit` 退出，线程也不会再运用线程私有数据 `destructors` 或清除处理函数。
这里就存在一个很危险的局面，其他线程可能正好位于临界区之内，持有了某个锁，而它突然死亡，也就再也没有机会去解锁了。而如果子进程试图去对某个 `mutex` 加锁，那么马上就会造成死锁的局面。

综上所述，除非你打算很快地在子进程中调用 `exec` 执行一个新程序， 否则应避免在一个多线程的程序中使用 `fork`。

如果你不能避免，那么你应该注意在子进程中不能调用以下这些函数（参考《Linux多线程服务端编程》）：

- `malloc`,会访问全局状态，肯定会加锁。
- 任何分配和释放内存的函数，诸如`new`、`delete`......
- 任何 Pthreads 函数。
- `printf` 系列函数。其他线程可能持有了stdout/stderr的锁。
- `sigle` 中除了“sigle安全”之外的任何函数，应避免在信号处理函数内使用 `fork`。

另外，Pthreads 增加了 `pthread_atfork` “fork 处理器” 机制以允许你的代码越过 `fork` 调用保护数据和不变量。

```c
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
```

该函数中，`prepare fork` 处理器以正确的顺序锁住所有的由相关代码使用的互斥量以阻止死锁的发生。`parent fork` 处理器只需要开锁所有互斥量即可，以允许父进程和所有线程继续正常工作。`child fork` 处理器经常可以与 parent fork 处评器一样； 但是有时需要重置程序或库的状态。

在程序 `atfork.c` 中展示了如何使用 `fork` 处理器。

## 多线程与exec

`exec` 函数功能是消除当前程序的环境并且用一个新程序代替它，所以并没有受线程的影响。

## 多线程与signal

在多线程程序中，使用 signal 的第一原则就是 **不要使用 signal**。

尽管修改进程信号行为本身是线程安全的，但是不能防止其他线程随后很快地设置一个新的信号行为。任何印象线程的信号同样也会影响整个进程，这意味着向进程或进程内的任何线程传送一个 `SIGKILL` 信号，将终止进程。传送一个 `SIGSTOP` 停止命令时，将导致所有的线程停止直到收到 `SIGCOUNT` 信号。

## 多线程与stdio

Pthreads 要求 ANSI C 标准 I/O (stdio) 函数是线程安全的。因为 stdio 包需要为输出缓冲区和文件状态指定静态存储区，stdio 实现将使用互斥量或信号灯等同步机制。

### flockfile 和 funlockfile

在一些情况里，一系列 stdio 操作以不被中断的顺序执行是重要的。例如，一个提示符后面跟着一个从终端的读操作。为了不让其他线程在两个操作之间读 stdin 或者写
stdout，你应该在两个调用前后锁住 stdin 和 stdou。你可以使用使用 `flockfile` 和 `funlockfile` 以及 `ftrylockfile` 函数来确保一系列写操作不会被从其他线程的文件存取打断。

```c
void flock file(FILE *file);
int  ftrylockfile(FILE *file)
void funlockfile(FILE *file)；
```

你可以参考程序 `flock.c` 实例。

### getchar_unlocked 和 putchar_unlocked

函数 `getchar` 和 `putchar` 分别操作 stdin 和 stdout，而 `getc` 和 `putc` 能在任何stdio 文件流上被使用。Pthreads 要求这些函数锁住 stdio 流数据来防止代码对 stdio 缓冲区的偶然破坏。

Phtread 提供了函数 `getc_unlocked`、`putc_unlocked`、 `getchar_unlocked` 和 `putchar_unlocked`，但它们不执行任何锁操作，因此你必须在这些操作附近使用 `flockfile` 和 `fimlockfili`。

```c
int getc_unlocked(FILE *stream);
int getchar_unlocked(void);
int putc_unlocked(int c, FILE *stream);
int putchar_unlocked (int c);
```

程序 `putchar.c` 显示了使用 `putchar` 和在一个文件锁内调用一系列 `putchar_unlocked`之间的差异。

## 进程结束

在一个多线程程序中，主函数是”进程主线程的启动函数”，从主函数返回将终止整个进程。与进程相关的所有内存（和线程）将消失。线程也不会执行清除处理器或线程私有数据 `destructors` 函数。调用 `exit` 具有同样的效果，你可以调用 `exit` 来很快地停止所有的线程。

当你不想使用起始线程或让它等待其他线程结束时，可以通过调用 `pthread_exit` 而非返回或调用 `exit` 退出主函数。从主函数中调用 `pthread_exit` 将在不影响进程内其他线程的前提下终止起始线程，允许其他线程继续运作，直到正常完成。

## 线程安全函数

Pthreads 定义了现存函数的线程安全的变体，它们在相应函数名结尾处添加后缀 `_r`：

- 用户和终端ID，`getlogin_r`、`ctermid`、`ttyname_r`。
- 目录搜索，`readdir_r`。
- 字符串 token，`strtok_r`。
- 时间表示，`asctime_r`、`ctime_r`、`gmtime_r`、`localtime_r`。
- 随机数产生，`read_r`。
- 组和用户数据库，`getgrgid_r`、`getgrnam_r`、`getpwuid_r`、`getpwnam_r`。

程序 `getlogin.c` 是一个如何调用获取用户和终端ID的实例。