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