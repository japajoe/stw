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

#ifndef STW_WEB_REQUEST_ROUTER_HPP
#define STW_WEB_REQUEST_ROUTER_HPP

#include "../net/http.hpp"
#include "../net/network_stream.hpp"
#include <vector>
#include <regex>
#include <functional>


namespace stw::experimental
{
	using web_request_handler = std::function<http_response(const http_request &request, network_stream *stream)>;

	class web_request_router
	{
	public:
		bool process_request(const http_request &request, network_stream *stream, http_response &response);
		void add(http_method method, const std::string &route, web_request_handler handler);
		void add(http_method method, const std::regex &route, web_request_handler handler);
	private:
		struct http_route
		{
			std::regex regex;
			http_method method;
			web_request_handler requestHandler;
		};

		std::vector<http_route> routes;
		http_route *get(const std::string &route);
	};
}

#endif