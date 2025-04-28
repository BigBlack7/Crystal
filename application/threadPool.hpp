#pragma once
#include <thread>
#include <functional> // 用于封装可调用对象
#include <vector>
#include <queue>
#include "spinLock.hpp"
/*
    线程池是一种并发编程模型，用于管理一组预先创建的线程，这些线程可以执行提交给线程池的任务。这种模型可以避免频繁创建和销毁线程带来的开销，提高程序的性能。
*/
class Task
{
public:
    virtual void run() = 0;
    virtual ~Task() = default;
};

class ThreadPool
{
public:
    static void workerThread(ThreadPool *master); // 工作线程的入口点，每个工作线程都会执行这个函数

    ThreadPool(size_t threadCount = 0); // 创建指定数量的工作线程，threadCount=0时默认使用硬件并发数量
    ~ThreadPool();                      // 等待所有任务完成并销毁所有线程

    // 并行执行一个二维循环，将每个循环迭代作为一个任务提交给线程池
    void parallelFor(size_t width, size_t height, const std::function<void(size_t, size_t)> &lambda, bool _isComplex = true);
    // 等待所有任务完成
    void wait() const;

    void addTask(Task *task); // 添加一个任务到线程池
    Task *getTask();          // 从任务队列中获取一个任务

private:
    std::atomic<int> mAlive;            // 线程池是否存活的标志
    std::vector<std::thread> mThreads;  // 线程池中的线程
    std::atomic<int> mPendingTaskCount; // 待处理任务的数量
    std::queue<Task *> mTasks;          // 任务队列，用于存储待执行的任务。
    SpinLock mSpinLock{};               // 用于保护任务队列的自旋锁，确保线程安全。应当有出队入队两个锁，否则只能主线程操作。
};

extern ThreadPool threadPool; // 全局线程池实例，用于简化线程池的使用。