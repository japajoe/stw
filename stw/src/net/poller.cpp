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
#include <unordered_map>
#else // Linux
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/eventfd.h>
#endif

#include <iostream>

namespace stw
{
	// --- LINUX (epoll) ---
#if defined(__linux__)
	class epoll_poller : public poller
	{
	public:
		epoll_poller()
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

		~epoll_poller()
		{
			if (notifyFD != -1)
				close(notifyFD);
			if (epollFD != -1)
				close(epollFD);
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
		void notify() override
		{
			uint64_t signal = 1;
			// Writing to eventfd increments its internal counter and wakes epoll
			write(notifyFD, &signal, sizeof(signal));
		}

		int32_t wait(std::vector<poll_event_result> &results, int32_t timeout) override
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

				if (revents[i].events & EPOLLIN)
					res.flags |= poll_event_read;
				if (revents[i].events & EPOLLOUT)
					res.flags |= poll_event_write;
				if (revents[i].events & EPOLLERR)
					res.flags |= poll_event_error;
				if (revents[i].events & EPOLLHUP)
					res.flags |= poll_event_disconnect;

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
			ev.events = 0; // Level triggered

			if (flags & poll_event_read)
				ev.events |= EPOLLIN;
			if (flags & poll_event_write)
				ev.events |= EPOLLOUT;

			return epoll_ctl(epollFD, op, fd, &ev) == 0;
		}
	};
#elif defined(__APPLE__) || defined(__FreeBSD__)
	class kqueue_poller : public poller
	{
	public:
		kqueue_poller() : fd_to_idx(65536, -1) // Pre-allocate for max FDs
		{
			events.resize(1024);
			kqueueFD = kqueue();

			struct kevent kev;
			EV_SET(&kev, 0, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, nullptr);
			kevent(kqueueFD, &kev, 1, nullptr, 0, nullptr);
		}

        ~kqueue_poller()
        {
            if (kqueueFD != -1)
                close(kqueueFD);
        }

        bool add(int fd, poll_event_flag flags) override
        {
            std::lock_guard<std::mutex> lock(mtx);
            // Use EV_ADD | EV_ENABLE for new descriptors
            return ctl(fd, flags, EV_ADD | EV_ENABLE);
        }

        bool modify(int32_t fd, poll_event_flag flags) override
        {
            std::lock_guard<std::mutex> lock(mtx);
            // In kqueue, EV_ADD updates existing filters. 
            // Our ctl helper handles deleting unused filters.
            return ctl(fd, flags, EV_ADD | EV_ENABLE);
        }

        bool remove(int32_t fd) override
        {
            std::lock_guard<std::mutex> lock(mtx);
            struct kevent kev[2];
            // We ignore errors here because we don't know if both READ and WRITE existed
            EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
            EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
            kevent(kqueueFD, kev, 2, nullptr, 0, nullptr);
            return true;
        }

        void notify() override
        {
            struct kevent kev;
            // Trigger the user event registered in the constructor
            EV_SET(&kev, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0, nullptr);
            kevent(kqueueFD, &kev, 1, nullptr, 0, nullptr);
        }

		int32_t wait(std::vector<poll_event_result> &results, int32_t timeout_ms) override
		{
			struct timespec ts;
			struct timespec *timeout_ptr = nullptr;
			if (timeout_ms >= 0) {
				ts.tv_sec = timeout_ms / 1000;
				ts.tv_nsec = (timeout_ms % 1000) * 1000000;
				timeout_ptr = &ts;
			}

			// 1. Get events from kernel
			int32_t n = kevent(kqueueFD, nullptr, 0, events.data(), events.size(), timeout_ptr);
			
			// 2. Process events using Direct Indexing (No Map!)
			for (int i = 0; i < n; ++i)
			{
				if (events[i].filter == EVFILT_USER) continue;

				int32_t fd = static_cast<int32_t>(events[i].ident);
				
				// Boundary check for safety
				if (fd >= (int)fd_to_idx.size()) fd_to_idx.resize(fd + 1024, -1);

				int32_t idx = fd_to_idx[fd];
				if (idx != -1)
				{
					map_flags(events[i], results[idx]);
				}
				else
				{
					fd_to_idx[fd] = static_cast<int32_t>(results.size());
					poll_event_result res = {fd, 0};
					map_flags(events[i], res);
					results.push_back(res);
				}
			}

			// 3. Reset the lookup array for the NEXT call efficiently
			for (const auto& res : results) {
				fd_to_idx[res.fd] = -1;
			}

			return static_cast<int32_t>(results.size());
		}

	private:
		int32_t kqueueFD;
		std::vector<struct kevent> events;
		
		// CHANGE: Use a vector as a Direct Address Table (O(1) lookup, no hashing)
		std::vector<int32_t> fd_to_idx; 
		std::mutex mtx;

		// ... (map_flags is fine) ...

		bool ctl(int32_t fd, uint32_t flags, uint16_t action)
		{
			// PERFORMANCE FIX: Don't call kevent() multiple times inside ctl.
			// Prepare one batch and send it in one system call.
			struct kevent kev[2];
			int n = 0;

			// Add/Update Read
			if (flags & poll_event_read)
				EV_SET(&kev[n++], fd, EVFILT_READ, action, 0, 0, nullptr);
			else
				EV_SET(&kev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);

			// Add/Update Write
			if (flags & poll_event_write)
				EV_SET(&kev[n++], fd, EVFILT_WRITE, action, 0, 0, nullptr);
			else
				EV_SET(&kev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

			// We ignore EV_DELETE errors here because a filter might not exist
			return kevent(kqueueFD, kev, n, nullptr, 0, nullptr) != -1;
		}
	};

    // class kqueue_poller : public poller
    // {
    // public:
    //     kqueue_poller()
    //     {
    //         events.resize(1024);
    //         kqueueFD = kqueue();

    //         struct kevent kev;
    //         EV_SET(&kev, 0, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, nullptr);
    //         kevent(kqueueFD, &kev, 1, nullptr, 0, nullptr);
    //     }

    //     ~kqueue_poller()
    //     {
    //         if (kqueueFD != -1)
    //             close(kqueueFD);
    //     }

    //     bool add(int fd, poll_event_flag flags) override
    //     {
    //         std::lock_guard<std::mutex> lock(mtx);
    //         // Use EV_ADD | EV_ENABLE for new descriptors
    //         return ctl(fd, flags, EV_ADD | EV_ENABLE);
    //     }

    //     bool modify(int32_t fd, poll_event_flag flags) override
    //     {
    //         std::lock_guard<std::mutex> lock(mtx);
    //         // In kqueue, EV_ADD updates existing filters. 
    //         // Our ctl helper handles deleting unused filters.
    //         return ctl(fd, flags, EV_ADD | EV_ENABLE);
    //     }

    //     bool remove(int32_t fd) override
    //     {
    //         std::lock_guard<std::mutex> lock(mtx);
    //         struct kevent kev[2];
    //         // We ignore errors here because we don't know if both READ and WRITE existed
    //         EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    //         EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    //         kevent(kqueueFD, kev, 2, nullptr, 0, nullptr);
    //         return true;
    //     }

    //     void notify() override
    //     {
    //         struct kevent kev;
    //         // Trigger the user event registered in the constructor
    //         EV_SET(&kev, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0, nullptr);
    //         kevent(kqueueFD, &kev, 1, nullptr, 0, nullptr);
    //     }

    //     int32_t wait(std::vector<poll_event_result> &results, int32_t timeout_ms) override
    //     {
    //         struct timespec ts;
    //         struct timespec *timeout_ptr = nullptr;
    //         if (timeout_ms >= 0) {
    //             ts.tv_sec = timeout_ms / 1000;
    //             ts.tv_nsec = (timeout_ms % 1000) * 1000000;
    //             timeout_ptr = &ts;
    //         }

    //         int32_t n = kevent(kqueueFD, nullptr, 0, events.data(), events.size(), timeout_ptr);
            
    //         // To match epoll behavior, results should likely be cleared by the caller 
    //         // or here, depending on your API contract. 
    //         fd_to_idx.clear();

    //         for (int i = 0; i < n; ++i)
    //         {
    //             if (events[i].filter == EVFILT_USER)
    //                 continue;

    //             int32_t fd = static_cast<int32_t>(events[i].ident);

    //             auto it = fd_to_idx.find(fd);
    //             if (it != fd_to_idx.end())
    //             {
    //                 map_flags(events[i], results[it->second]);
    //             }
    //             else
    //             {
    //                 poll_event_result res = {fd, 0};
    //                 map_flags(events[i], res);
    //                 fd_to_idx[fd] = results.size();
    //                 results.push_back(res);
    //             }
    //         }
    //         return static_cast<int32_t>(results.size());
    //     }

    // private:
    //     int32_t kqueueFD;
    //     std::vector<struct kevent> events;
    //     std::unordered_map<int32_t, size_t> fd_to_idx;
    //     std::mutex mtx;

    //     void map_flags(const struct kevent &ev, poll_event_result &res)
    //     {
    //         if (ev.filter == EVFILT_READ) res.flags |= poll_event_read;
    //         if (ev.filter == EVFILT_WRITE) res.flags |= poll_event_write;
    //         if (ev.flags & EV_ERROR) res.flags |= poll_event_error;
    //         if (ev.flags & EV_EOF) res.flags |= poll_event_disconnect;
    //     }

    //     bool ctl(int32_t fd, uint32_t flags, uint16_t action)
    //     {
    //         struct kevent kev[2];
    //         int n = 0;

    //         if (flags & poll_event_read)
    //             EV_SET(&kev[n++], fd, EVFILT_READ, action, 0, 0, nullptr);
    //         else {
    //             // Remove the read filter if it was there
    //             struct kevent del_kev;
    //             EV_SET(&del_kev, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    //             kevent(kqueueFD, &del_kev, 1, nullptr, 0, nullptr);
    //         }

    //         if (flags & poll_event_write)
    //             EV_SET(&kev[n++], fd, EVFILT_WRITE, action, 0, 0, nullptr);
    //         else {
    //             // Remove the write filter if it was there
    //             struct kevent del_kev;
    //             EV_SET(&del_kev, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    //             kevent(kqueueFD, &del_kev, 1, nullptr, 0, nullptr);
    //         }

    //         if (n == 0) return true;
    //         return kevent(kqueueFD, kev, n, nullptr, 0, nullptr) != -1;
    //     }
    // };
#elif defined(_WIN32) || defined(_WIN64)
	class wsa_poller : public poller
	{
	public:
		wsa_poller()
		{
			// Windows doesn't have eventfd; we use a local socket pair for the notify signal
			setup_notify_sockets();
		}

		~wsa_poller()
		{
			if (notifySend != INVALID_SOCKET)
				closesocket(notifySend);
			if (notifyRecv != INVALID_SOCKET)
				closesocket(notifyRecv);
		}

		bool add(int32_t fd, poll_event_flag flags) override
		{
			std::lock_guard<std::mutex> lock(mtx);
			WSAPOLLFD pfd = {};
			pfd.fd = static_cast<SOCKET>(fd);
			pfd.events = translate_flags_to_win(flags);
			pollFDs.push_back(pfd);
			dirty = true;
			return true;
		}

		bool modify(int32_t fd, poll_event_flag flags) override
		{
			std::lock_guard<std::mutex> lock(mtx);
			for (auto &pfd : pollFDs)
			{
				if (pfd.fd == static_cast<SOCKET>(fd))
				{
					pfd.events = translate_flags_to_win(flags);
					dirty = true;
					return true;
				}
			}
			return false;
		}

		bool remove(int32_t fd) override
		{
			std::lock_guard<std::mutex> lock(mtx);
			auto it = std::remove_if(pollFDs.begin(), pollFDs.end(), [fd](const WSAPOLLFD &pfd)
									 { return pfd.fd == static_cast<SOCKET>(fd); });
			if (it != pollFDs.end())
			{
				pollFDs.erase(it, pollFDs.end());
				dirty = true;
				return true;
			}
			return false;
		}

		void notify() override
		{
			char signal = 1;
			send(notifySend, &signal, 1, 0);
		}

		int32_t wait(std::vector<poll_event_result> &results, int32_t timeout) override
		{
			// Only copy if the interest list actually changed
			{
				std::lock_guard<std::mutex> lock(mtx);
				if (dirty)
				{
					workingSet = pollFDs; // std::vector assignment reuses existing memory
					dirty = false;
				}
			}

			if (workingSet.empty())
				return 0;

			int ret = WSAPoll(workingSet.data(), static_cast<ULONG>(workingSet.size()), timeout);

			if (ret > 0)
			{
				for (auto &pfd : workingSet)
				{
					if (pfd.revents == 0)
						continue;

					if (pfd.fd == notifyRecv)
					{
						char dummy;
						recv(notifyRecv, &dummy, 1, 0); // Drain UDP datagram
						continue;
					}

					poll_event_result res;
					res.fd = static_cast<int32_t>(pfd.fd);
					res.flags = 0;

					if (pfd.revents & POLLIN)
						res.flags |= poll_event_read;
					if (pfd.revents & POLLOUT)
						res.flags |= poll_event_write;
					if (pfd.revents & POLLERR)
						res.flags |= poll_event_error;
					if (pfd.revents & POLLHUP)
						res.flags |= poll_event_disconnect;

					results.push_back(res);
				}
			}
			return static_cast<int32_t>(results.size());
		}

	private:
		SOCKET notifySend = INVALID_SOCKET;
		SOCKET notifyRecv = INVALID_SOCKET;
		std::vector<WSAPOLLFD> pollFDs;
		std::vector<WSAPOLLFD> workingSet;
		bool dirty = true;
		std::mutex mtx;

		void setup_notify_sockets()
		{
			// 1. Create two UDP sockets
			notifySend = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			notifyRecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

			sockaddr_in addr = {};
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			addr.sin_port = 0; // Let OS choose a random port

			// 2. Bind the receiver to a local port
			bind(notifyRecv, (sockaddr *)&addr, sizeof(addr));

			// 3. Find out which port the OS assigned to the receiver
			int len = sizeof(addr);
			getsockname(notifyRecv, (sockaddr *)&addr, &len);

			// 4. "Connect" the sender to that specific port
			// This allows us to use send() instead of sendto()
			connect(notifySend, (sockaddr *)&addr, sizeof(addr));

			// 5. Put the receiver in the poll list
			WSAPOLLFD pfd = {};
			pfd.fd = notifyRecv;
			pfd.events = POLLIN;

			std::lock_guard<std::mutex> lock(mtx);
			pollFDs.push_back(pfd);
			dirty = true; // Ensure the working set picks this up
		}

		short translate_flags_to_win(poll_event_flag flags)
		{
			short win_flags = 0;
			if (flags & poll_event_read)
				win_flags |= POLLIN;
			if (flags & poll_event_write)
				win_flags |= POLLOUT;
			return win_flags;
		}
	};
#endif

	// --- Factory ---
	std::unique_ptr<poller> poller::create()
	{
#if defined(__linux__)
		return std::make_unique<epoll_poller>();
#elif defined(__APPLE__) || defined(__FreeBSD__)
		return std::make_unique<kqueue_poller>();
#elif defined(_WIN32) || defined(_WIN64)
		return std::make_unique<wsa_poller>();
#else
		throw std::runtime_error("stw::poller for unknown platform not implemented");
#endif
	}
}
