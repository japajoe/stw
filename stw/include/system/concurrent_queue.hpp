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

#ifndef STW_CONCURRENT_QUEUE_HPP
#define STW_CONCURRENT_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

namespace stw
{
    template <typename T>
    class concurrent_queue
    {
    public:
        void enqueue(const T &item)
        {
            std::lock_guard<std::mutex> lock(mutex);
            items.push(item);
            condition.notify_one();
        }

        bool try_dequeue(T &item)
        {
            std::lock_guard<std::mutex> lock(mutex);

            if (items.empty())
                return false;

            item = items.front();
            items.pop();
            return true;
        }

        void clear()
        {
            std::lock_guard<std::mutex> lock(mutex);
            while (!items.empty()) 
            {
                items.pop();
            }
        }

        std::size_t size() const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return items.size();
        }
    private:
        std::queue<T> items;
        mutable std::mutex mutex;
        std::condition_variable condition;
    };
}

#endif