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

#ifndef STW_NETWORK_STREAM_HPP
#define STW_NETWORK_STREAM_HPP

#include "socket.hpp"
#include <cstdint>
#include <cstdlib>
#include <memory>

namespace stw
{
	class network_stream
	{
	public:
		network_stream();
		network_stream(std::shared_ptr<stw::socket> socket, void *initialContent, uint64_t initialContentLength);
		~network_stream();
		int64_t read(void *buffer, size_t size);
		int64_t write(const void *buffer, size_t size);
	private:
		std::shared_ptr<stw::socket> socket;
		void *initialContent;
		uint64_t initialContentLength;
		uint64_t initialContentConsumed;
	};
}

#endif