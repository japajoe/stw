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

#ifndef STW_HTTP_CONNECTION_HPP
#define STW_HTTP_CONNECTION_HPP

#include "socket.hpp"
#include "ssl.hpp"
#include "http.hpp"
#include "../system/stream.hpp"
#include <cstdint>

namespace stw
{
	class http_connection
	{
	public:
		http_connection(socket &connection);
		http_connection(socket &connection, ssl &s);
		http_connection(const http_connection &other) = delete;
		http_connection(http_connection &&other) noexcept;
		http_connection &operator=(const http_connection &other) = delete;
		http_connection &operator=(http_connection &&other) noexcept;
		int64_t read(void *buffer, size_t size);
		int64_t write(const void *buffer, size_t size);
		int64_t peek(void *buffer, size_t size);
		int64_t read_all(void *buffer, size_t size);
		int64_t write_all(const void *buffer, size_t size);
		bool write_response(uint32_t statusCode);
		bool write_response(uint32_t statusCode, const http_headers *headers);
		bool write_response(uint32_t statusCode, const http_headers *headers, const void *content, uint64_t contentLength, const std::string &contentType);
		bool write_response(uint32_t statusCode, const http_headers *headers, stream *content, const std::string &contentType);
		bool write_response(uint32_t statusCode, const http_response_info *responseInfo);
		void close();
		bool is_secure() const;
		std::string get_ip() const;
	private:
		socket connection;
		ssl s;
	};
}

#endif