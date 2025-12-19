// MIT License
// Copyright © 2025 W.M.R Jap-A-Joe

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "thread_pool.hpp"

namespace stw
{
	thread_pool::thread_pool()
	{
		stopFlag.store(false);

		size_t numThreads = std::thread::hardware_concurrency();

		for (size_t i = 0; i < numThreads; ++i) 
		{
			workers.push_back(std::thread(&thread_pool::worker_thread, this));
		}

		activeThreads = 0;
	}

	thread_pool::~thread_pool()
	{
		stopFlag.store(true);
		cv.notify_all();
		
		for (auto &worker : workers) 
		{
			if (worker.joinable())
				worker.join();
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