# POSIX 多线程程序设计

## [概述](./doc/01_overview.md)

- [术语定义](./doc/01_overview.md#术语定义)
- [线程的好处](./doc/01_overview.md#线程的好处)
- [线程的代价](./doc/01_overview.md#线程的代价)
- [选择线程还是不用线程](./doc/01_overview.md#选择线程还是不用线程)
- [POSIX 线程概念](./doc/01_overview.md#POSIX线程概念)
- [异步编程举例](./doc/01_overview.md#异步编程举例)

## [线程](./doc/02_threads.md)

- [建立和使用线程](./doc/02_threads.md#建立和使用线程)
- [线程生命周期](./doc/02_threads.md#线程生命周期)

## [同步](./doc/03_synchronization.md)

- [不变量、临界区和谓词](./doc/03_synchronization.md#不变量、临界区和谓词)
- [互斥量](./doc/03_synchronization.md#互斥量)
- [条件变量](./doc/03_synchronization.md#条件变量)

## [使用线程方式](./doc/04_a_few_ways_to_threads.md)

- [流水线](./doc/04_a_few_ways_to_threads.md#流水线)
- [工作组](./doc/04_a_few_ways_to_threads.md#工作组)
- [客户/服务器](./doc/04_a_few_ways_to_threads.md#客户/服务器)

## [线程高级编程](./doc/05_advanced_thread_programming.md)

- [一次性初始化](./doc/05_advanced_thread_programming.md#一次性初始化)
- [线程属性](./doc/05_advanced_thread_programming.md#线程属性)
- [取消取消](./doc/05_advanced_thread_programming.md#取消取消)
- [线程私有数据](./doc/05_advanced_thread_programming.md#线程私有数据)
- [线程实时调度](./doc/05_advanced_thread_programming.md#线程实时调度)

## [POSIX 针对线程的调整](./doc/06_posix_adjusts_to_threads.md)

- [多线程与fork](./doc/06_posix_adjusts_to_threads.md#多线程与fork)
- [多线程与exec](./doc/06_posix_adjusts_to_threads.md#多线程与exec)
- [多线程与signal](./doc/06_posix_adjusts_to_threads.md#多线程与signal)
- [多线程与stdio](./doc/06_posix_adjusts_to_threads.md#多线程与stdio)
- [进程结束](./doc/06_posix_adjusts_to_threads.md#进程结束)
- [线程安全函数](./doc/06_posix_adjusts_to_threads.md#线程安全函数)

## [线程扩展](./doc/07_extended.md)

- [栅栏（Barriers）](./doc/07_extended.md#栅栏（Barriers）)
- [读/写锁（Read-Write Lock）](./doc/07_extended.md#读/写锁（Read-Write Lock）)
- [自旋锁（Spin Locks）](./doc/07_extended.md#自旋锁（Spin Locks）)
- [信号量（Semaphore）](./doc/07_extended.md#信号量（Semaphore）)

## [线程同步精要](./doc/08_synchronization_essentials.md)

- [线程同步四项原则](./doc/08_synchronization_essentials.md#线程同步四项原则)
- [互斥器](./doc/08_synchronization_essentials.md#互斥器)
- [条件变量](./doc/08_synchronization_essentials.md#条件变量)
- [不要使用读写锁和信号量](./doc/08_synchronization_essentials.md#不要使用读写锁和信号量)