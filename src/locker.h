/**
 * ## RAII
 * - RAII全称是“Resource Acquisition is Initialization”，直译过来是“资源获取即初始化”.
 * - RAII在构造函数中申请分配资源，在析构函数中释放资源。
 * 因为C++的语言机制保证了，当一个对象创建的时候，自动调用构造函数，当对象超出作用域的时候会自动调用析构函数。
 * 所以，在RAII的指导下，我们应该使用**类**来管理资源，将资源和对象的生命周期绑定。
 * - **RAII的核心思想是将资源或者状态与对象的生命周期绑定**，通过C++的语言机制，实现资源和状态的安全管理，智能指针是RAII最好的例子。
 * 
 * ## 信号量
 * 信号量是一种特殊的变量，它只能取自然数值并且只支持两种操作：等待(P)和信号(V)。
 * 假设有信号量SV，对其的P、V操作如下：
 * - P，如果SV的值大于0，则将其减一；若SV的值为0，则挂起执行
 * - V，如果有其他进行因为等待SV而挂起，则唤醒；若没有，则将SV值加一
 * 信号量的取值可以是任何自然数，最常用的，最简单的信号量是二进制信号量，只有0和1两个值.
 * - sem_init函数用于初始化一个未命名的信号量
 * - sem_destory函数用于销毁信号量
 * - sem_wait函数函数将以原子操作方式将信号量减一,信号量为0时,sem_wait阻塞
 * - sem_post函数以原子操作方式将信号量加一,信号量大于0时,唤醒调用sem_post的线程
 * 以上，成功返回0，失败返回errno
 * 
 * ## 互斥量（Mutex）
 * 互斥锁,也成互斥量,可以保护关键代码段,以确保独占式访问.当进入关键代码段,获得互斥锁将其加锁;离开关键代码段,唤醒等待该互斥锁的线程.
 * - pthread_mutex_init函数用于初始化互斥锁
 * - pthread_mutex_destory函数用于销毁互斥锁
 * - pthread_mutex_lock函数以原子操作方式给互斥锁加锁
 * - pthread_mutex_unlock函数以原子操作方式给互斥锁解锁
 * 
 * ## 条件变量
 * 条件变量提供了一种线程间的通知机制,当某个共享数据达到某个值时,唤醒等待这个共享数据的线程.
 * - pthread_cond_init函数用于初始化条件变量
 * - pthread_cond_destory函数销毁条件变量
 * - pthread_cond_wait函数用于等待目标条件变量.该函数调用时需要传入 mutex参数(加锁的互斥锁) ,
 * 函数执行时,先把调用线程放入条件变量的请求队列,然后将互斥锁mutex解锁,
 * 当函数成功返回为0时,互斥锁会再次被锁上.也就是说函数内部会有一次解锁和加锁操作.
 * 
 * ## 原子操作
 * 原子操作是指不可被中断的一个或一系列操作，要么全部执行成功，要么全部不执行。
 * 在多线程编程中，原子操作可以保证多个线程对同一数据的操作不会出现竞争条件，从而避免了数据不一致的问题。
 * 
 * ## 竞争条件
 * 竞争条件是指多个线程或进程在对共享资源进行访问时，由于执行顺序不确定或者执行时间不确定，导致最终结果的正确性无法保证的情况。
 * 竞争条件可能会导致数据不一致、死锁、饥饿等问题。
 * 在多线程编程中，为了避免竞争条件，需要使用同步机制，如互斥量、信号量、原子操作等。
 */

#pragma once

/**
 * <pthread.h>用于在程序中使用POSIX线程库（也称为pthread库）。
 * POSIX线程库是一套标准的多线程API，提供了创建、同步、销毁线程的函数，以及互斥量、条件变量等同步机制。
 * 
 * <semaphore.h>用于在程序中使用信号量。
 */
#include <exception>
#include <pthread.h>
#include <semaphore.h>

// 信号量（semaphore）
class sem
{
public:
    // 构造函数
    sem()
    {
        /**
         * 信号量初始化
         * 函数原型：int sem_init(sem_t *sem, int pshared, unsigned int value);
         * - sem是指向信号量的指针
         * - pshared指定信号量的类型
         * - value指定信号量的初值
         * 如果pshared为0，则信号量只能在当前进程的多个线程之间共享；如果pshared为非0，则信号量可以在多个进程之间共享。
         */
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }

    // 析构函数
    ~sem()
    {
        // 信号量销毁
        sem_destroy(&m_sem);
    }

    // 等待(P)，如果SV的值大于0，则将其减一；若SV的值为0，则挂起执行
    bool wait()
    {
        // sem_wait将以原子操作方式将信号量减一,信号量为0时,sem_wait阻塞
        return sem_wait(&m_sem) == 0;
    }

    // 信号(V)，如果有其他进行因为等待SV而挂起，则唤醒；若没有，则将SV值加一
    bool post()
    {
        // sem_post将以原子操作方式将信号量加一,信号量大于0时,唤醒调用sem_post的线程
        return sem_post(&m_sem) == 0;
    }

private:
    // 信号量
    sem_t m_sem;
};

// 互斥量（Mutex）
class locker
{
public:
    // 构造函数
    locker()
    {
        // 互斥锁初始化
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    // 析构函数
    ~locker()
    {
        // 互斥锁销毁
        pthread_mutex_destroy(&m_mutex);
    }

    bool lock()
    {
        // 互斥锁加锁
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool unlock()
    {
        // 互斥锁解锁
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    // 获取互斥锁
    pthread_mutex_t* get()
    {
        return &m_mutex;
    }

private:
    // 互斥锁
    pthread_mutex_t m_mutex;
};

// 条件变量（conditional variable）
class cond
{
public:
    cond()
    {
        // 条件变量初始化
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            // pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }

    /**
     * 条件变量的使用机制需要配合锁来使用
     * 内部会有一次加锁和解锁
     * 封装起来会使得更加简洁
     */
    bool wait(pthread_mutex_t* m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        
        /**
         * pthread_cond_wait是一个函数，用于等待条件变量的信号。
         * 函数原型：int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
         * - cond是指向条件变量的指针
         * - mutex是指向互斥量的指针
         * 在调用pthread_cond_wait之前，必须先对互斥量进行加锁，否则会出现竞争条件。
         * pthread_cond_wait会自动将当前线程加入到条件变量的等待队列中，并释放互斥量，使得其他线程可以访问共享资源。
         * 当条件变量被其他线程发出信号时，pthread_cond_wait会重新获得互斥量，并返回。
         * 如果在等待过程中出现错误，pthread_cond_wait会返回一个非0的错误码。
         */
        ret = pthread_cond_wait(&m_cond, m_mutex);
        
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    bool timewait(pthread_mutex_t* m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);

        /**
         * pthread_cond_timedwait和pthread_cond_wait不同之处在于pthread_cond_timedwait可以设置等待的超时时间，
         * 如果超过了指定的时间仍然没有收到信号，就会返回一个错误码。
         */
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);

        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    bool signal()
    {
        /**
         * pthread_cond_signal用于发送条件变量的信号。
         * 函数原型：int pthread_cond_signal(pthread_cond_t *cond);
         * - cond是指向条件变量的指针。
         * pthread_cond_signal会唤醒等待在条件变量上的**一个**线程，使其从等待状态中返回。
         */
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadcast()
    {
        /**
         * pthread_cond_broadcast用于广播条件变量的信号。
         * 函数原型：int pthread_cond_broadcast(pthread_cond_t *cond);
         * - cond是指向条件变量的指针。
         * pthread_cond_broadcast会唤醒等待在条件变量上的**所有**线程，使其从等待状态中返回。
         */
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    // static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};