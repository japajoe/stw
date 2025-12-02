#ifndef STW_THREAD_POOL_HPP
#define STW_THREAD_POOL_HPP

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <cstdlib>

namespace stw
{
	class thread_pool
	{
	public:
		thread_pool();
	    thread_pool(size_t numThreads);
    	~thread_pool();
		void enqueue(std::function<void()> task);
		bool is_available() const;
	private:
		std::vector<std::thread> workers;
		std::queue<std::function<void()>> taskQueue;
		std::mutex queueMutex;
		std::condition_variable cv;
		std::atomic<bool> stopFlag;
		std::atomic<size_t> activeThreads;
		void worker_thread();
	};
}

#endif