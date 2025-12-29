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

#include "http.hpp"
#include "http_stream.hpp"

namespace stw
{
	class http_controller
	{
	public:
		virtual ~http_controller() = default;
		virtual http_response on_get(http_request &request, http_stream *stream);
		virtual http_response on_post(http_request &request, http_stream *stream);
		virtual http_response on_put(http_request &request, http_stream *stream);
		virtual http_response on_patch(http_request &request, http_stream *stream);
		virtual http_response on_delete(http_request &request, http_stream *stream);
		virtual http_response on_head(http_request &request, http_stream *stream);
		virtual http_response on_options(http_request &request, http_stream *stream);
		virtual http_response on_trace(http_request &request, http_stream *stream);
		virtual http_response on_connect(http_request &request, http_stream *stream);
		virtual http_response on_unknown_method(http_request &request, http_stream *stream);
	protected:
		inline http_response create_response(uint32_t statusCode);
	};
}

#endif