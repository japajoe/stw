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

#ifndef STW_HTTP_SERVER
#define STW_HTTP_SERVER

#include "socket.hpp"
#include "ssl.hpp"
#include "http.hpp"
#include "http_connection.hpp"
#include "http_config.hpp"
#include "../system/thread_pool.hpp"
#include <string>
#include <functional>
#include <vector>
#include <atomic>

namespace stw
{
	using http_request_handler = std::function<void(http_connection *connection, const http_request &request)>;

	class http_server
	{
	public:
		http_request_handler onRequest;
		http_server();
		~http_server();
		void run(const http_config &config);
		void stop();
	private:
		std::vector<socket> listeners;
		ssl_context sslContext;
		http_config config;
		std::atomic<bool> quit;
		thread_pool threadPool;
		void accept_http();
		void accept_https();
		void on_request(http_connection *connection);
		http_header_error read_header(http_connection *connection, std::string &header);
		bool parse_request_header(const std::string &responseText, http_headers &header, std::string &method, std::string &path, uint64_t &contentLength);
	};
}

#endif