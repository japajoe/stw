#include "stw_thread_pool.hpp"

namespace stw
{
	thread_pool::thread_pool()
	{
		auto numThreads = std::thread::hardware_concurrency();

		for (size_t i = 0; i < numThreads; ++i) 
		{
			workers.push_back(std::thread(&thread_pool::worker_thread, this));
		}

		activeThreads = 0;
	}

	thread_pool::thread_pool(size_t numThreads)
	{
		for (size_t i = 0; i < numThreads; ++i) 
		{
			workers.push_back(std::thread(&thread_pool::worker_thread, this));
		}

		activeThreads = 0;
	}

	thread_pool::~thread_pool()
	{
		stopFlag = true;
		cv.notify_all();
		
		for (auto &worker : workers) 
		{
			if (worker.joinable())
				worker.join();  // Wait for each thread to finish
		}
	}

	void thread_pool::enqueue(std::function<void()> task)
	{
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			taskQueue.push(task);
		}
		cv.notify_one();  // Notify one thread that a task is available
	}

	void thread_pool::worker_thread()
	{
		while (!stopFlag) 
		{
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				cv.wait(lock, [this] { 
					return !taskQueue.empty() || stopFlag; 
				});
				
				if (stopFlag && taskQueue.empty())
					return;

				task = taskQueue.front();
				taskQueue.pop();
			}
			
			activeThreads++;
			task();
			activeThreads--;
		}
	}

	bool thread_pool::is_available() const
	{
		return activeThreads < workers.size();
	}
}