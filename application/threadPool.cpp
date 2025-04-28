#include "threadPool.hpp"
#include <cmath>

ThreadPool threadPool{};
// 线程池中的工作线程函数，每个工作线程都会执行该函数。该函数会不断地从任务队列中取出任务并执行，直到线程池被销毁。
void ThreadPool::workerThread(ThreadPool *master)
{
	while (master->mAlive == 1) // 检查线程池是否处于活动状态
	{
		if (master->mTasks.empty()) // 如果任务队列为空，线程会休眠2ms，避免空转CPU
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
			continue;
		}

		// 从任务队列中取出一个任务并执行
		Task *task = master->getTask();
		if (task != nullptr)
		{
			task->run();
			delete task;
			master->mPendingTaskCount--;
		}
		else
		{
			// 获取到空任务就把它的操作权交给os，线程会让出 CPU 时间片，让其他线程有机会执行。
			std::this_thread::yield();
		}
	}
}

// 创建指定数量的工作线程，threadCount=0时默认使用硬件并发数量
ThreadPool::ThreadPool(size_t threadCount)
{
	mAlive = 1;			   // 线程池处于活动状态
	mPendingTaskCount = 0; // 初始化待处理任务的数量为0
	if (threadCount == 0)
	{
		threadCount = std::thread::hardware_concurrency(); // 获取硬件并发数量
	}
	for (size_t i = 0; i < threadCount; i++)
	{
		mThreads.push_back(std::thread(ThreadPool::workerThread, this)); // 创建工作线程并将其加入线程池
	}
}

ThreadPool::~ThreadPool()
{
	wait();		// 阻塞当前线程，直到所有任务完成
	mAlive = 0; // 线程池不再处于活动状态
	for (auto &thread : mThreads)
	{
		thread.join(); // 等待工作线程完成
	}
	mThreads.clear(); // 清空线程池
}

class ParallelForTask : public Task
{
public:
	ParallelForTask(size_t x, size_t y, size_t chunckWidth, size_t chunckHeight, std::function<void(size_t, size_t)> lambda)
		: x(x), y(y), chunckWidth(chunckWidth), chunckHeight(chunckHeight), lambda(lambda) {}
	// 实现任务的具体逻辑，通过嵌套循环遍历指定的区域，并调用 lambda 函数处理每个元素。
	void run() override
	{
		for (size_t idx_x = 0; idx_x < chunckWidth; idx_x++)
		{
			for (size_t idx_y = 0; idx_y < chunckHeight; idx_y++)
			{
				lambda(x + idx_x, y + idx_y);
			}
		}
	}

private:
	size_t x, y, chunckWidth, chunckHeight;
	std::function<void(size_t, size_t)> lambda; // 存储要执行的任务函数
};

void ThreadPool::parallelFor(size_t width, size_t height, const std::function<void(size_t, size_t)> &lambda, bool _isComplex)
{
	Guard guard(mSpinLock); // 确保在当前函数执行期间，对共享资源（如任务队列）的访问是线程安全的

	// 任务分块, 解决每一个像素都要new一个任务出来造成巨大性能损耗的问题
	float chunckWidthF = (float)(static_cast<float>(width) / std::sqrt(mThreads.size()));
	float chunckHeightF = (float)(static_cast<float>(height) / std::sqrt(mThreads.size()));
	if (_isComplex) // 复杂任务, 进一步分块(任务内容确定，没有分支结构的任务是简单任务)
	{
		chunckWidthF /= (float)std::sqrt(16);
		chunckHeightF /= (float)std::sqrt(16);
	}

	size_t chunckWidth = static_cast<size_t>(std::ceil(chunckWidthF));
	size_t chunckHeight = static_cast<size_t>(std::ceil(chunckHeightF));

	for (size_t x = 0; x < width; x += chunckWidth)
	{
		for (size_t y = 0; y < height; y += chunckHeight)
		{
			mPendingTaskCount++;		 // 增加待处理任务的数量
			if (x + chunckWidth > width) // 处理边界情况，确保任务不超出边界
			{
				chunckWidth = width - x;
			}
			if (y + chunckHeight > height)
			{
				chunckHeight = height - y;
			}
			// 创建一个 ParallelForTask 任务，并将其添加到任务队列中。
			mTasks.push(new ParallelForTask(x, y, chunckWidth, chunckHeight, lambda));
		}
	}
}

void ThreadPool::wait() const
{
	// 等待所有任务完成，不断检查待处理任务的数量，直到为0。
	while (mPendingTaskCount > 0)
	{
		std::this_thread::yield();
	}
}

void ThreadPool::addTask(Task *task)
{
	Guard guard(mSpinLock); // 确保在当前函数执行期间，对共享资源（如任务队列）的访问是线程安全的
	mPendingTaskCount++;
	mTasks.push(task);
}

Task *ThreadPool::getTask()
{
	Guard guard(mSpinLock);
	if (mTasks.empty())
	{
		return nullptr;
	}
	Task *task = mTasks.front(); // 如果任务队列不为空，获取队列中第一个任务的指针
	mTasks.pop();				 // 从任务队列中移除第一个任务
	return task;
}
