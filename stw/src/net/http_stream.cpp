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

#include "http_stream.hpp"
#include <cstring>

namespace stw
{
	http_stream::http_stream()
	{
		socket = nullptr;
		initialContent = nullptr;
		initialContentLength = 0;
		initialContentConsumed = 0;
	}

	http_stream::http_stream(std::shared_ptr<stw::socket> socket, void *initialContent, size_t initialContentLength)
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

	http_stream::~http_stream()
	{
		if(initialContent)
			std::free(initialContent);
		initialContent = nullptr;
	}

	int64_t http_stream::read(void *buffer, size_t size)
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

	bool http_stream::read_as_string(std::string &str, uint64_t size)
	{
		if (size == 0) 
			return false;

		str.reserve(str.size() + size);

        char buffer[4096];
        uint64_t bytesRemaining = size;

        while (bytesRemaining > 0)
        {
            size_t toRead = (bytesRemaining < sizeof(buffer)) ? bytesRemaining : sizeof(buffer);

            int64_t bytesRead = this->read(buffer, toRead);

            if (bytesRead <= 0)
            {
				if(STW_SOCKET_ERR == STW_EAGAIN || STW_SOCKET_ERR == STW_EWOULDBLOCK)
					continue;
				return false;
            }

            str.append(buffer, static_cast<size_t>(bytesRead));
            bytesRemaining -= static_cast<uint64_t>(bytesRead);
        }

        return true;
	}
}