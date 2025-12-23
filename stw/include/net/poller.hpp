#ifndef STW_POLLER_HPP
#define STW_POLLER_HPP

#include <vector>
#include <cstdint>
#include <memory>

namespace stw
{
	enum class poll_event : uint32_t
	{
		read = 0x01,
		write = 0x02,
		error = 0x04
	};

	struct poll_event_result
	{
		int fd;
		uint32_t events;
	};

	class poller
	{
	public:
		virtual ~poller() = default;
		virtual bool add(int fd, uint32_t events) = 0;
		virtual bool modify(int fd, uint32_t events) = 0;
		virtual bool remove(int fd) = 0;
		virtual int wait(std::vector<poll_event_result> &results, int timeOut) = 0;
		// Factory method
		static std::unique_ptr<poller> create();
	};
}
#endif