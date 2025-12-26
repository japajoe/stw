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

#include "poller.hpp"
#include <algorithm>
#include <vector>
#include <mutex>

#if defined(_WIN32) || defined(_WIN64)
	#ifdef _WIN32_WINNT
	#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600
	#include <winsock2.h>
	#include <ws2tcpip.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
	#include <sys/types.h>
	#include <sys/event.h>
	#include <sys/time.h>
	#include <unistd.h>
#else // Linux
	#include <sys/epoll.h>
	#include <unistd.h>
	#include <sys/eventfd.h>
#endif

namespace stw
{
	// --- LINUX (epoll) ---
#if defined(__linux__)
	class linux_poller : public poller
	{
	public:
		linux_poller()
		{
			revents.resize(1024);
			epollFD = epoll_create1(0);
			// EFD_NONBLOCK ensures the read/write doesn't hang the worker
			notifyFD = eventfd(0, EFD_NONBLOCK);

			// Internal registration of the notifyFD so it can wake up wait()
			struct epoll_event ev;
			ev.data.fd = notifyFD;
			ev.events = EPOLLIN; // Level-triggered is fine for the interruptor
			epoll_ctl(epollFD, EPOLL_CTL_ADD, notifyFD, &ev);
		}

		~linux_poller()
		{
			if (notifyFD != -1) close(notifyFD);
        	if (epollFD != -1) close(epollFD);
		}

		bool add(int fd, poll_event_flag flags) override
		{
			std::lock_guard<std::mutex> lock(mtx);
			return ctl(EPOLL_CTL_ADD, fd, flags);
		}

		bool modify(int32_t fd, poll_event_flag flags) override 
		{
			std::lock_guard<std::mutex> lock(mtx);
			return ctl(EPOLL_CTL_MOD, fd, flags);
		}

		bool remove(int32_t fd) override
		{
			std::lock_guard<std::mutex> lock(mtx);
			return epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, nullptr) == 0;
		}

		// Trigger the poller to wake up from wait()
		void notify()
		{
			uint64_t signal = 1;
			// Writing to eventfd increments its internal counter and wakes epoll
			write(notifyFD, &signal, sizeof(signal));
		}

		int32_t wait(std::vector<poll_event_result> &results, int32_t timeout)
		{
			int nfds = epoll_wait(epollFD, revents.data(), revents.size(), timeout);
			
			for (int32_t i = 0; i < nfds; ++i) 
			{
				if (revents[i].data.fd == notifyFD) 
				{
					// Drain the eventfd notification
					uint64_t dummy;
					read(notifyFD, &dummy, sizeof(dummy));
					continue; 
				}

				poll_event_result res;
				res.fd = revents[i].data.fd;
				res.flags = 0;

				if (revents[i].events & EPOLLIN)  res.flags |= poll_event_read;
				if (revents[i].events & EPOLLOUT) res.flags |= poll_event_write;
				if (revents[i].events & EPOLLERR) res.flags |= poll_event_error;
				if (revents[i].events & EPOLLHUP) res.flags |= poll_event_disconnect;

				results.push_back(res);
			}
			return static_cast<int32_t>(results.size());
		}

	private:
		int32_t epollFD;
		int32_t notifyFD;
		std::vector<struct epoll_event> revents;
		std::mutex mtx;

		bool ctl(int32_t op, int32_t fd, uint32_t flags) 
		{
			struct epoll_event ev;
			ev.data.fd = fd;
			ev.events = 0;  // Level triggered
			
			if (flags & poll_event_read)  ev.events |= EPOLLIN;
			if (flags & poll_event_write) ev.events |= EPOLLOUT;
			
			return epoll_ctl(epollFD, op, fd, &ev) == 0;
		}
	};
#endif

	// --- Factory ---
	std::unique_ptr<poller> poller::create()
	{
#if defined(__linux__)
		return std::make_unique<linux_poller>();
#elif defined(__APPLE__) || defined(__FreeBSD__)
		std::runtime_error("stw::poller for Mac and FreeBSD not yet implemented");
#elif defined(_WIN32) || defined(_WIN64)
		std::runtime_error("stw::poller for Windows not yet implemented");
#else
		std::runtime_error("stw::poller for unknown platform not implemented");
#endif
	}
}
