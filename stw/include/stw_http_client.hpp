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

#ifndef STW_HTTP_CLIENT_HPP
#define STW_HTTP_CLIENT_HPP

#include "stw_http.hpp"
#include <string>
#include <cstdint>
#include <unordered_map>
#include <functional>

namespace stw
{
	class http_client
	{
	public:
		http_response_callback onResponse;
		http_client();
		bool get(const http_request &req, http_response &res);
		bool post(const http_request &req, http_response &res);
		void set_validate_certificate(bool validate);
		bool validate_certificate() const;
	private:
		bool validateCertificate;
		static bool parse_header(const std::string &responseText, http_headers &header, int &statusCode, uint64_t &contentLength);
		static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
		static size_t header_callback(void *contents, size_t size, size_t nmemb, void *userp);
	};
}

#endif