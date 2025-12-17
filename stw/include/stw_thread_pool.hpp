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