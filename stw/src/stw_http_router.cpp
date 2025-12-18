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

#include "stw_http_router.hpp"

namespace stw
{
	void http_router::add(http_method method, const std::string &route, http_request_handler handler)
	{
		http_route r = {
			.regex = std::regex(route),
			.method = method,
			.requestHandler = handler,
			.controllerHandler = nullptr};

		routes.push_back(r);
	}

	http_route *http_router::get(const std::string &route)
	{
		for (auto &r : routes)
		{
			std::smatch match;

			if (std::regex_match(route, match, r.regex))
				return &r;
		}

		return nullptr;
	}

	bool http_router::process_request(const std::string &route, stw::http_connection *connection, const stw::http_request_info &request)
	{
		auto r = get(route);

		if (!r)
			return false;

		if (r->requestHandler)
		{
			if (r->method == request.method)
				r->requestHandler(connection, request);
			else
				connection->write_response(stw::http_status_code_method_not_allowed);
		}
		else
		{
			if (r->controllerHandler)
			{
				auto controller = r->controllerHandler();
				
				if (controller)
				{
					switch (request.method)
					{
					case http_method_get:
						controller->on_get(connection, request);
						break;
					case http_method_post:
						controller->on_get(connection, request);
						break;
					case http_method_put:
						controller->on_get(connection, request);
						break;
					case http_method_patch:
						controller->on_get(connection, request);
						break;
					case http_method_delete:
						controller->on_get(connection, request);
						break;
					case http_method_head:
						controller->on_get(connection, request);
						break;
					case http_method_options:
						controller->on_get(connection, request);
						break;
					case http_method_trace:
						controller->on_get(connection, request);
						break;
					case http_method_connect:
						controller->on_get(connection, request);
						break;
					default:
						controller->on_unknown_method(connection, request);
					}
				}
				else
				{
					// Out of memory
					connection->write_response(stw::http_status_code_internal_server_error);
				}
			}
			else
			{
				// This shouldn't actually happen
				connection->write_response(stw::http_status_code_internal_server_error);
			}
		}
		return true;
	}
}