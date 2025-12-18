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

#include "stw_http_connection.hpp"
#include <utility>
#include <sstream>

namespace stw
{
	http_connection::http_connection(socket &connection)
	{
		this->connection = std::move(connection);
	}

	http_connection::http_connection(socket &connection, ssl &s)
	{
		this->connection = std::move(connection);
		this->s = std::move(s);
	}

	http_connection::http_connection(http_connection &&other) noexcept
	{
		connection = std::move(other.connection);
		s = std::move(other.s);
	}

	http_connection &http_connection::operator=(http_connection &&other) noexcept
	{
		if(this != &other)
		{
			connection = std::move(other.connection);
			s = std::move(other.s);
		}
		return *this;
	}

	int64_t http_connection::read(void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.read(buffer, size);
		return connection.read(buffer, size);
	}

	int64_t http_connection::write(const void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.write(buffer, size);
		return connection.write(buffer, size);
	}

	int64_t http_connection::peek(void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.peek(buffer, size);
		return connection.peek(buffer, size);
	}

	int64_t http_connection::read_all(void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.read_all(buffer, size);
		return connection.read_all(buffer, size);
	}

	int64_t http_connection::write_all(const void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.write_all(buffer, size);
		return connection.write_all(buffer, size);
	}

	bool http_connection::write_response(uint32_t statusCode)
	{
		return write_response(statusCode, nullptr, nullptr, 0, "");
	}

	bool http_connection::write_response(uint32_t statusCode, const http_headers *headers)
	{
		return write_response(statusCode, headers, nullptr, 0, "");
	}

	bool http_connection::write_response(uint32_t statusCode, const http_headers *headers, stream *content, const std::string &contentType)
	{
		std::ostringstream responseText;
		responseText << "HTTP/1.1 " << statusCode << "\r\n";
		
		if(headers)
		{
			if(headers->size() > 0)
			{
				for(const auto &[key,value] : *headers)
				{
					responseText << key << ": " << value << "\r\n";
				}
			}
		}

		if(content != nullptr && contentType.size() > 0)
		{
			responseText << "Content-Type: " << contentType << "\r\n";
			responseText << "Content-Length: " << content->get_length() << "\r\n";
		}

		responseText << "Connection: close\r\n\r\n";

		if(write_all(responseText.str().data(), responseText.str().size()))
		{
			if(content != nullptr && content->get_length() > 0)
			{
				std::vector<uint8_t> buffer(8192);

				int64_t nBytes = 0;
				
				while((nBytes = content->read(buffer.data(), buffer.size())) > 0)
				{
					write(buffer.data(), nBytes);
				}
			}
			return true;
		}
		return false;
	}

	bool http_connection::write_response(uint32_t statusCode, const http_headers *headers, const void *content, uint64_t contentLength, const std::string &contentType)
	{
		std::ostringstream responseText;
		responseText << "HTTP/1.1 " << statusCode << "\r\n";

		if(headers)
		{
			if(headers->size() > 0)
			{
				for(const auto &[key,value] : *headers)
				{
					responseText << key << ": " << value << "\r\n";
				}
			}
		}

		if(content != nullptr && contentLength > 0 && contentType.size() > 0)
		{
			responseText << "Content-Type: " << contentType << "\r\n";
			responseText << "Content-Length: " << contentLength << "\r\n";
		}

		responseText << "Connection: close\r\n\r\n";

		if(write_all(responseText.str().data(), responseText.str().size()))
		{
			if(content != nullptr && contentLength > 0)
				return write_all(content, contentLength);
			return true;
		}
		return false;
	}

	bool http_connection::write_response(uint32_t statusCode, const http_response_info *responseInfo)
	{
		if(!responseInfo)
			return false;
		return write_response(statusCode, &responseInfo->headers, responseInfo->content.data(), responseInfo->content.size(), responseInfo->contentType);
	}

	void http_connection::close()
	{
		connection.close();
		s.destroy();
	}

	bool http_connection::is_secure() const
	{
		return s.is_valid();
	}

	std::string http_connection::get_ip() const
	{
		return connection.get_ip();
	}
}