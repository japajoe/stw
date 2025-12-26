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

#include "network_stream.hpp"
#include <cstring>

namespace stw
{
	network_stream::network_stream()
	{
		socket = nullptr;
		initialContent = nullptr;
		initialContentLength = 0;
		initialContentConsumed = 0;
	}

	network_stream::network_stream(std::shared_ptr<stw::socket> socket, void *initialContent, size_t initialContentLength)
	{
		this->socket = socket;
		
		if(!initialContent || initialContentLength == 0)
		{
			this->initialContent = nullptr;
			this->initialContentLength = 0;
			initialContentConsumed = 0;
		}
		else
		{
			this->initialContent = std::malloc(initialContentLength);
			this->initialContentLength = initialContentLength;

			std::memcpy(this->initialContent, initialContent, initialContentLength);

			initialContentConsumed = 0;
		}
	}

	network_stream::~network_stream()
	{
		if(initialContent)
			std::free(initialContent);
		initialContent = nullptr;
	}

	int64_t network_stream::read(void *buffer, size_t size)
	{
		if (socket == nullptr) 
			return 0;

		if (buffer == nullptr) 
			return 0;

		if (size == 0) 
			return 0;

		if(initialContentLength == 0)
			return socket->read(buffer, size);

		if (initialContentConsumed < initialContentLength)
		{
			size_t remaining = initialContentLength - initialContentConsumed;
			size_t toConsume = (size < remaining) ? size : remaining;

			std::memcpy(buffer, (uint8_t*)initialContent + initialContentConsumed, toConsume);
			initialContentConsumed += toConsume;

			return static_cast<int64_t>(toConsume);
		}

		return socket->read(buffer, size);
	}

	int64_t network_stream::write(const void *buffer, size_t size)
	{
		if(socket == nullptr)
			return 0;
		return socket->write(buffer, size);
	}
}