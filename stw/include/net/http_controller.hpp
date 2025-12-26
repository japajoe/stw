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

#ifndef STW_HTTP_CONTROLLER_HPP
#define STW_HTTP_CONTROLLER_HPP

#include "http_connection.hpp"
#include "http.hpp"
#include <sstream>
#include <string>
#include <unordered_map>

namespace stw
{
	class http_controller
	{
	public:
		virtual ~http_controller() = default;
		virtual void on_get(http_connection *connection, const http_request &request);
		virtual void on_post(http_connection *connection, const http_request &request);
		virtual void on_put(http_connection *connection, const http_request &request);
		virtual void on_patch(http_connection *connection, const http_request &request);
		virtual void on_delete(http_connection *connection, const http_request &request);
		virtual void on_head(http_connection *connection, const http_request &request);
		virtual void on_options(http_connection *connection, const http_request &request);
		virtual void on_trace(http_connection *connection, const http_request &request);
		virtual void on_connect(http_connection *connection, const http_request &request);
		virtual void on_unknown_method(http_connection *connection, const http_request &request);
	};
}

#endif