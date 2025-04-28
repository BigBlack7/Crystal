#pragma once
#include <thread>
#include <atomic> // 原子操作
/*
    自旋锁是一种忙等待锁，线程在尝试获取锁时会不断循环检查锁的状态，直到锁被释放。锁守卫类用于自动管理自旋锁的获取和释放，确保在作用域结束时锁会被正确释放。
*/
class SpinLock // 自旋锁
{
public:
    // 尝试获取锁
    void acquire()
    {
        // 设置标志并返回其之前的值。如果标志之前已经被设置，则线程会让出 CPU 时间片，继续循环等待，直到标志被清除
        while (mFlag.test_and_set(std::memory_order_acquire))
        {
            // 线程让出 CPU 时间片，继续循环等待
            std::this_thread::yield();
        }
    }

    // 释放锁
    void release()
    {
        // 清除标志位，允许其他线程获取锁
        mFlag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag mFlag{}; // 原子标志位，表示锁的状态
};

// 守卫类，用于自动获取和释放锁
class Guard
{
public:
    Guard(SpinLock &spinLock) : mSpinLock(spinLock) { mSpinLock.acquire(); }
    ~Guard() { mSpinLock.release(); }

private:
    SpinLock &mSpinLock; // 管理自旋锁⬆
};