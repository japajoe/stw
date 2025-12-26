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

#include "web_request_router.hpp"

namespace stw::experimental
{
	void web_request_router::add(http_method method, const std::string &route, web_request_handler handler)
	{
		add(method, std::regex(route), handler);
	}

	void web_request_router::add(http_method method, const std::regex &route, web_request_handler handler)
	{
		if(!handler)
			throw std::runtime_error("request handler must be set");

		http_route r = {
			.regex = route,
			.method = method,
			.requestHandler = handler
		};

		routes.push_back(r);
	}

	web_request_router::http_route *web_request_router::get(const std::string &route)
	{
		for (auto &r : routes)
		{
			std::smatch match;

			if (std::regex_match(route, match, r.regex))
				return &r;
		}

		return nullptr;
	}

	bool web_request_router::process_request(const http_request &request, network_stream *stream, http_response &response)
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
		else
		{
				response.content = nullptr;
				response.statusCode = stw::http_status_code_not_found;
		}

		return true;
	}
}