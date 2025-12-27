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

#include "http_controller.hpp"

namespace stw
{
	http_response http_controller::on_get(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_post(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_put(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_patch(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_delete(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_head(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_options(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_trace(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_connect(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_method_not_allowed);
	}

	http_response http_controller::on_unknown_method(const http_request &request, http_stream *stream)
	{
		return create_response(http_status_code_not_implemented);
	}

	http_response http_controller::create_response(uint32_t statusCode)
	{
		http_response response;
		response.content = nullptr;
		response.statusCode = statusCode;
		return response;
	}
}