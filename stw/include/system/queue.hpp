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

#ifndef STW_QUEUE_HPP
#define STW_QUEUE_HPP

#include <atomic>
#include <vector>
#include <optional>

namespace stw
{
	//Single producer, single consumer queue
	template <typename T, size_t Size>
	class queue
	{
	public:
		bool enqueue(T value)
		{
			size_t t = tail.load(std::memory_order_relaxed);
			size_t next_t = (t + 1) & mask;

			if (next_t == head.load(std::memory_order_acquire))
			{
				return false; // Queue is full
			}

			buffer[t] = value;
			// Release ensures the data write above is visible before tail is updated
			tail.store(next_t, std::memory_order_release);
			return true;
		}

		// Worker calls this
		bool try_dequeue(T &outValue)
		{
			size_t h = head.load(std::memory_order_relaxed);

			// Acquire ensures we see the data written before the producer updated tail
			if (h == tail.load(std::memory_order_acquire))
			{
				return false; // Queue is empty
			}

			outValue = buffer[h];
			head.store((h + 1) & mask, std::memory_order_release);
			return true;
		}
	private:
		// Size must be a power of 2 for the mask trick to work
		static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");

		T buffer[Size];
		std::atomic<size_t> head{0};
		std::atomic<size_t> tail{0};
		static constexpr size_t mask = Size - 1;
	};
}

#endif