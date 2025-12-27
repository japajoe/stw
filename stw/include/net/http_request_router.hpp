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

#ifndef STW_HTTP_REQUEST_ROUTER_HPP
#define STW_HTTP_REQUEST_ROUTER_HPP

#include "http.hpp"
#include "http_stream.hpp"
#include "http_controller.hpp"
#include <vector>
#include <regex>
#include <memory>
#include <functional>
#include <type_traits>

namespace stw
{
	using http_request_handler = std::function<http_response(const http_request &request, http_stream *stream)>;
	using http_controller_handler = std::function<std::unique_ptr<http_controller>()>;

	class http_request_router
	{
	public:
		bool process_request(const http_request &request, http_stream *stream, http_response &response);
		void add(http_method method, const std::string &route, http_request_handler handler);
		void add(http_method method, const std::regex &route, http_request_handler handler);

		template <typename T>
		void add(const std::string &route) 
		{
            static_assert(std::is_base_of<http_controller, T>::value, "http_request_router::add parameter T must derive from http_controller");

			http_route r = {
				.regex = std::regex(route),
				.method = http_method_unknown,
				.requestHandler = nullptr,
				.controllerHandler = []() { return std::make_unique<T>(); }
			};

			routes.push_back(r);
		}
		
		template <typename T>
		void add(const std::regex &route) 
		{
            static_assert(std::is_base_of<http_controller, T>::value, "http_request_router::add parameter T must derive from http_controller");

			http_route r = {
				.regex = route,
				.method = http_method_unknown,
				.requestHandler = nullptr,
				.controllerHandler = []() { return std::make_unique<T>(); }
			};

			routes.push_back(r);
		}

	private:
		struct http_route
		{
			std::regex regex;
			http_method method;
			http_request_handler requestHandler;
			http_controller_handler controllerHandler;
		};

		std::vector<http_route> routes;
		http_route *get(const std::string &route);
	};
}

#endif