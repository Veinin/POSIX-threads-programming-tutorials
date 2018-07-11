# POSIX 多线程程序设计

本项目所有内容摘录自《POSIX多线程程序设计》与《Linux多线程服务端编程:使用muduoC++网络库》部分章节，简化了书中的内容，并提供了完整的源代码实现。

## 编译、运行

前提，你需要安装 CMake。所有章节源代码都放在 `src` 目录下，你可以直接编译后运行。

```shell
git clone https://github.com/Veinin/programming-with-POSIX-threads-tutorials.git
cd programming-with-POSIX-threads-tutorials
./build.sh
```

运行实例代码：

```shell
$ ./bin/barrier_main
00: (10)0000045001 0000045002 0000045003 0000045004 0000045005 0000045006
01: (11)0000055001 0000055002 0000055003 0000055004 0000055005 0000055006
02: (12)0000065001 0000065002 0000065003 0000065004 0000065005 0000065006
03: (13)0000075001 0000075002 0000075003 0000075004 0000075005 0000075006
04: (14)0000085001 0000085002 0000085003 0000085004 0000085005 0000085006
```

## 目录

* [概述](./doc/01_overview.md)
  * [术语定义](./doc/01_overview.md#术语定义)
  * [线程的好处](./doc/01_overview.md#线程的好处)
  * [线程的代价](./doc/01_overview.md#线程的代价)
  * [选择线程还是不用线程](./doc/01_overview.md#选择线程还是不用线程)
  * [POSIX 线程概念](./doc/01_overview.md#POSIX线程概念)
  * [异步编程举例](./doc/01_overview.md#异步编程举例)

* [线程](./doc/02_threads.md)
  * [建立和使用线程](./doc/02_threads.md#建立和使用线程)
  * [线程生命周期](./doc/02_threads.md#线程生命周期)

* [同步](./doc/03_synchronization.md)
  * [不变量、临界区和谓词](./doc/03_synchronization.md#不变量、临界区和谓词)
  * [互斥量](./doc/03_synchronization.md#互斥量)
  * [条件变量](./doc/03_synchronization.md#条件变量)

* [使用线程方式](./doc/04_a_few_ways_to_threads.md)
  * [流水线](./doc/04_a_few_ways_to_threads.md#流水线)
  * [工作组](./doc/04_a_few_ways_to_threads.md#工作组)
  * [客户/服务器](./doc/04_a_few_ways_to_threads.md#客户/服务器)

* [线程高级编程](./doc/05_advanced_thread_programming.md)
  * [一次性初始化](./doc/05_advanced_thread_programming.md#一次性初始化)
  * [属性](./doc/05_advanced_thread_programming.md#属性)
  * [取消](./doc/05_advanced_thread_programming.md#取消)
  * [线程私有数据](./doc/05_advanced_thread_programming.md#线程私有数据)
  * [线程实时调度](./doc/05_advanced_thread_programming.md#线程实时调度)

* [POSIX 针对线程的调整](./doc/06_posix_adjusts_to_threads.md)
  * [多线程与fork](./doc/06_posix_adjusts_to_threads.md#多线程与fork)
  * [多线程与exec](./doc/06_posix_adjusts_to_threads.md#多线程与exec)
  * [多线程与signal](./doc/06_posix_adjusts_to_threads.md#多线程与signal)
  * [多线程与stdio](./doc/06_posix_adjusts_to_threads.md#多线程与stdio)
  * [进程结束](./doc/06_posix_adjusts_to_threads.md#进程结束)
  * [线程安全函数](./doc/06_posix_adjusts_to_threads.md#线程安全函数)

* [线程扩展](./doc/07_extended.md)
  * [栅栏（Barriers）](./doc/07_extended.md#%E6%A0%85%E6%A0%8Fbarriers)
  * [读/写锁（Read-Write Lock）](./doc/07_extended.md#%E8%AF%BB%E5%86%99%E9%94%81read-write-lock)
  * [自旋锁（Spin Locks）](./doc/07_extended.md#%E8%87%AA%E6%97%8B%E9%94%81spin-locks)
  * [信号量（Semaphore）](./doc/07_extended.md##%E4%BF%A1%E5%8F%B7%E9%87%8Fsemaphore)
  * [工作队列](./doc/07_extended.md#工作队列)

* [线程同步精要](./doc/08_synchronization_essentials.md)
  * [线程同步四项原则](./doc/08_synchronization_essentials.md#线程同步四项原则)
  * [互斥器](./doc/08_synchronization_essentials.md#%E4%BA%92%E6%96%A5%E5%99%A8mutex)
  * [条件变量](./doc/08_synchronization_essentials.md#条件变量)
  * [不要使用读写锁和信号量](./doc/08_synchronization_essentials.md#不要使用读写锁和信号量)