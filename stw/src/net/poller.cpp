#include "poller.hpp"
#include <algorithm>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
	#include <winsock2.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
	#include <sys/types.h>
	#include <sys/event.h>
	#include <sys/time.h>
	#include <unistd.h>
#else // Linux
	#include <sys/epoll.h>
	#include <unistd.h>
#endif

namespace stw
{
	// --- LINUX (epoll) ---
#if defined(__linux__)
	class epoll_poller : public poller
	{
		int epoll_fd;

	public:
		epoll_poller() { epoll_fd = epoll_create1(0); }
		~epoll_poller()
		{
			if (epoll_fd >= 0)
				close(epoll_fd);
		}

		bool add(int fd, uint32_t events) override
		{
			struct epoll_event ev = {0};
			if (events & (uint32_t)poll_event::read)
				ev.events |= EPOLLIN;
			if (events & (uint32_t)poll_event::write)
				ev.events |= EPOLLOUT;
			ev.data.fd = fd;
			return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == 0;
		}

		bool modify(int fd, uint32_t events) override
		{
			struct epoll_event ev = {0};
			if (events & (uint32_t)poll_event::read)
				ev.events |= EPOLLIN;
			if (events & (uint32_t)poll_event::write)
				ev.events |= EPOLLOUT;
			ev.data.fd = fd;
			return epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev) == 0;
		}

		bool remove(int fd) override
		{
			return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr) == 0;
		}

		int wait(std::vector<poll_event_result> &results, int timeOut) override
		{
			int batchSize = results.capacity() > 0 ? results.capacity() : 64;
			auto raw = std::make_unique<struct epoll_event[]>(batchSize);
			int n = epoll_wait(epoll_fd, raw.get(), batchSize, timeOut);
			for (int i = 0; i < n; ++i)
			{
				uint32_t res = 0;
				if (raw[i].events & EPOLLIN)
					res |= (uint32_t)poll_event::read;
				if (raw[i].events & EPOLLOUT)
					res |= (uint32_t)poll_event::write;
				if (raw[i].events & (EPOLLERR | EPOLLHUP))
					res |= (uint32_t)poll_event::error;
				results.push_back({raw[i].data.fd, res});
			}
			return n;
		}
	};

	// --- MAC/BSD (kqueue) ---
#elif defined(__APPLE__) || defined(__FreeBSD__)
	class kqueue_poller : public poller
	{
		int kq;

	public:
		kqueue_poller() { kq = kqueue(); }
		~kqueue_poller()
		{
			if (kq >= 0)
				close(kq);
		}

		bool add(int fd, uint32_t events) override
		{
			struct kevent kev[2];
			int n = 0;
			if (events & (uint32_t)poll_event::read)
				EV_SET(&kev[n++], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void *)(intptr_t)fd);
			if (events & (uint32_t)poll_event::write)
				EV_SET(&kev[n++], fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, (void *)(intptr_t)fd);
			return kevent(kq, kev, n, NULL, 0, NULL) != -1;
		}

		bool modify(int fd, uint32_t events) override
		{
			remove(fd); // kqueue modification is often cleanest as remove/add
			return add(fd, events);
		}

		bool remove(int fd) override
		{
			struct kevent kev[2];
			EV_SET(&kev[0], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
			EV_SET(&kev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
			return kevent(kq, kev, 2, NULL, 0, NULL) != -1;
		}

		int wait(std::vector<poll_event_result> &results, int timeout_ms) override
		{
			int batchSize = results.capacity() > 0 ? results.capacity() : 64;
			auto raw = std::make_unique<struct kevent[]>(batchSize);
			struct timespec ts = {timeout_ms / 1000, (timeout_ms % 1000) * 1000000};
			int n = kevent(kq, NULL, 0, raw.get(), batchSize, &ts);
			for (int i = 0; i < n; ++i)
			{
				uint32_t res = 0;
				if (raw[i].filter == EVFILT_READ)
					res |= (uint32_t)poll_event::read;
				if (raw[i].filter == EVFILT_WRITE)
					res |= (uint32_t)poll_event::write;
				if (raw[i].flags & EV_EOF)
					res |= (uint32_t)poll_event::error;
				results.push_back({(int)(intptr_t)raw[i].udata, res});
			}
			return n;
		}
	};

	// --- WINDOWS (WSAPoll) ---
#elif defined(_WIN32) || defined(_WIN64)
	class wsa_poller : public poller
	{
		std::vector<WSAPOLLFD> poll_fds;

	public:
		bool add(int fd, uint32_t events) override
		{
			WSAPOLLFD pfd = {0};
			pfd.fd = (SOCKET)fd;
			if (events & (uint32_t)poll_event::read)
				pfd.events |= POLLRDNORM;
			if (events & (uint32_t)poll_event::write)
				pfd.events |= POLLWRNORM;
			poll_fds.push_back(pfd);
			return true;
		}

		bool modify(int fd, uint32_t events) override
		{
			for (auto &pfd : poll_fds)
			{
				if (pfd.fd == (SOCKET)fd)
				{
					pfd.events = 0;
					if (events & (uint32_t)poll_event::read)
						pfd.events |= POLLRDNORM;
					if (events & (uint32_t)poll_event::write)
						pfd.events |= POLLWRNORM;
					return true;
				}
			}
			return false;
		}

		bool remove(int fd) override
		{
			auto it = std::remove_if(poll_fds.begin(), poll_fds.end(),
									[fd](const WSAPOLLFD &pfd)
									{ return pfd.fd == (SOCKET)fd; });
			if (it != poll_fds.end())
			{
				poll_fds.erase(it, poll_fds.end());
				return true;
			}
			return false;
		}

		int wait(std::vector<poll_event_result> &results, int timeout_ms) override
		{
			if (poll_fds.empty())
				return 0;
			int n = WSAPoll(poll_fds.data(), (ULONG)poll_fds.size(), timeout_ms);
			if (n > 0)
			{
				for (auto &pfd : poll_fds)
				{
					if (pfd.revents != 0)
					{
						uint32_t res = 0;
						if (pfd.revents & POLLRDNORM)
							res |= (uint32_t)poll_event::read;
						if (pfd.revents & POLLWRNORM)
							res |= (uint32_t)poll_event::write;
						if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
							res |= (uint32_t)poll_event::error;
						results.push_back({(int)pfd.fd, res});
					}
				}
			}
			return n;
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
	#endif
	}
}
