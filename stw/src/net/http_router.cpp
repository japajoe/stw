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

#include "http_router.hpp"

namespace stw
{
	void http_router::add(http_method method, const std::string &route, http_request_handler handler)
	{
		add(method, std::regex(route), handler);
	}

	void http_router::add(http_method method, const std::regex &route, http_request_handler handler)
	{
		if(!handler)
			throw std::runtime_error("request handler must be set");

		routes.emplace_back(route, method, handler, nullptr);
	}

	http_router::http_route *http_router::get(const std::string &route)
	{
		for (auto &r : routes)
		{
			std::smatch match;

			if (std::regex_match(route, match, r.regex))
				return &r;
		}

		return nullptr;
	}

	bool http_router::process_request(const http_request &request, http_stream *stream, http_response &response)
	{
		auto r = get(request.path);

		if (!r)
			return false;

		if (r->requestHandler)
		{
			if (r->method == request.method)
			{
				response = r->requestHandler(request, stream);
			}
			else
			{
				response.content = nullptr;
				response.statusCode = stw::http_status_code_method_not_allowed;
			}
		}
		else if(r->controllerHandler)
		{
			auto controller = std::unique_ptr<http_controller>(r->controllerHandler());
			
			if (controller)
			{
				switch (request.method)
				{
				case http_method_get:
					response = controller->on_get(request, stream);
					break;
				case http_method_post:
					response = controller->on_post(request, stream);
					break;
				case http_method_put:
					response = controller->on_put(request, stream);
					break;
				case http_method_patch:
					response = controller->on_patch(request, stream);
					break;
				case http_method_delete:
					response = controller->on_delete(request, stream);
					break;
				case http_method_head:
					response = controller->on_head(request, stream);
					break;
				case http_method_options:
					response = controller->on_options(request, stream);
					break;
				case http_method_trace:
					response = controller->on_trace(request, stream);
					break;
				case http_method_connect:
					response = controller->on_connect(request, stream);
					break;
				default:
					response = controller->on_unknown_method(request, stream);
				}
			}
			else
			{
				// Out of memory
				response.content = nullptr;
				response.statusCode = stw::http_status_code_internal_server_error;
			}
		}
		else
		{
			// Getting here shouldn't actually ever happen
			response.content = nullptr;
			response.statusCode = stw::http_status_code_not_found;
		}

		return true;
	}
}