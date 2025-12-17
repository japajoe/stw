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

#ifndef STW_HTTP_ROUTER_HPP
#define STW_HTTP_ROUTER_HPP

#include "stw_http.hpp"
#include "stw_http_server.hpp"
#include <vector>
#include <regex>
#include <functional>

namespace stw
{
	using http_request_handler = std::function<void(stw::http_connection *connection, const stw::http_request_info &request)>;

	struct http_route
	{
		std::regex regex;
		http_method method;
		http_request_handler handler;
	};
	
	class http_router
	{
	public:
		void add(http_method method, const std::string &route, http_request_handler handler);
		http_route *get(const std::string &route);
	private:
		std::vector<http_route> routes;
	};
}

#endif