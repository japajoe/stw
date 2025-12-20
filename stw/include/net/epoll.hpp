#ifndef STW_EPOLL_HPP
#define STW_EPOLL_HPP

#include "../core/platform.hpp"
#include <cstdint>

#if defined(STW_PLATFORM_LINUX)
	#include <sys/epoll.h>
#endif

#if defined(STW_PLATFORM_LINUX)
namespace stw::epoll
{
	int32_t create(int32_t size);
	int32_t create1(int32_t flags);
	int32_t close(int32_t fd);
	int32_t ctl(int32_t epfd, int32_t op, int32_t fd, struct epoll_event *ev);
	int32_t wait(int32_t epfd, struct epoll_event *events, int32_t maxEvents, int32_t timeOut);
}
#elif defined(STW_PLATFORM_WINDOWS)
enum EPOLL_EVENTS 
{
	EPOLLIN      = (int) (1U <<  0),
	EPOLLPRI     = (int) (1U <<  1),
	EPOLLOUT     = (int) (1U <<  2),
	EPOLLERR     = (int) (1U <<  3),
	EPOLLHUP     = (int) (1U <<  4),
	EPOLLRDNORM  = (int) (1U <<  6),
	EPOLLRDBAND  = (int) (1U <<  7),
	EPOLLWRNORM  = (int) (1U <<  8),
	EPOLLWRBAND  = (int) (1U <<  9),
	EPOLLMSG     = (int) (1U << 10), /* Never reported. */
	EPOLLRDHUP   = (int) (1U << 13),
	EPOLLONESHOT = (int) (1U << 31)
};

#define EPOLLIN      (1U <<  0)
#define EPOLLPRI     (1U <<  1)
#define EPOLLOUT     (1U <<  2)
#define EPOLLERR     (1U <<  3)
#define EPOLLHUP     (1U <<  4)
#define EPOLLRDNORM  (1U <<  6)
#define EPOLLRDBAND  (1U <<  7)
#define EPOLLWRNORM  (1U <<  8)
#define EPOLLWRBAND  (1U <<  9)
#define EPOLLMSG     (1U << 10)
#define EPOLLRDHUP   (1U << 13)
#define EPOLLONESHOT (1U << 31)

#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_MOD 2
#define EPOLL_CTL_DEL 3

typedef void* HANDLE;
typedef uintptr_t SOCKET;

typedef union epoll_data 
{
	void* ptr;
	int fd;
	uint32_t u32;
	uint64_t u64;
	SOCKET sock; /* Windows specific */
	HANDLE hnd;  /* Windows specific */
} epoll_data_t;

struct epoll_event 
{
	uint32_t events;   /* Epoll events and flags */
	epoll_data_t data; /* User data variable */
};

namespace stw::epoll
{
	HANDLE create(int32_t size);
	HANDLE create1(int32_t flags);
	int32_t close(HANDLE ephnd);
	int32_t ctl(HANDLE ephnd, int op, SOCKET sock, struct epoll_event* event);
	int32_t wait(HANDLE ephnd, struct epoll_event* events, int maxEvents, int timeOut);
}
#endif //STW_PLATFORM_WINDOWS

#endif