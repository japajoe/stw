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

#ifndef STW_POLLER_HPP
#define STW_POLLER_HPP

#include <vector>
#include <cstdint>
#include <memory>

namespace stw
{
	enum poll_event_flag
	{
		poll_event_read = 0x01,
		poll_event_write = 0x02,
		poll_event_error = 0x04,
		poll_event_disconnect = 0x08 // Maybe useful for things other than non fatal error?
	};

	struct poll_event_result 
	{
		int32_t fd;
		uint32_t flags;
	};

	class poller
	{
	public:
		virtual ~poller() {}
		virtual bool add(int32_t fd, poll_event_flag flags) = 0;
		virtual bool remove(int32_t fd) = 0;
		virtual bool modify(int32_t fd, poll_event_flag flags) = 0;
		virtual void notify() = 0;
		virtual int32_t wait(std::vector<poll_event_result> &results, int32_t timeout) = 0;
		static std::unique_ptr<poller> create();
	};
}
#endif