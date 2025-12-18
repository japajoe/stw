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

#ifndef STW_HTTP_HPP
#define STW_HTTP_HPP

#include <string>
#include <cstdint>
#include <unordered_map>
#include <functional>

namespace stw
{
	using http_headers = std::unordered_map<std::string,std::string>;

    class http_request
    {
    public:
        http_request();
        ~http_request();
        void set_url(const std::string &url);
        std::string get_url() const;
        void set_content(void *data, uint64_t size, bool copyData = false);
        uint8_t *get_content() const;
        uint64_t get_content_length() const;
        void set_content_type(const std::string &contentType);
        std::string get_content_type() const;
        void set_header(const std::string &key, const std::string &value);
        http_headers get_headers() const;
    private:
        std::string url;
        uint8_t *content;        
        uint64_t contentLength;
        std::string contentType;
        http_headers header;
        bool ownsData;
    };
    
    class http_client;

    class http_response
    {
    friend class http_client;
    public:
        http_response();
        int get_status_code() const;
        uint64_t get_content_length() const;
        http_headers &get_headers();
    private:
        int32_t statusCode;
        uint64_t contentLength;
        http_headers header;
    };

    enum http_header_error 
    {
        http_header_error_none,
        http_header_error_failed_to_peek,
        http_header_error_failed_to_read,
        http_header_error_end_not_found,
        http_header_error_max_size_exceeded
    };

    enum http_method 
    {
        http_method_get,
        http_method_post,
        http_method_put,
        http_method_patch,
        http_method_delete,
        http_method_head,
        http_method_options,
        http_method_trace,
        http_method_connect,
        http_method_unknown
    };

    enum http_status_code 
    {
        http_status_code_ok = 200,
        http_status_code_created = 201,
        http_status_code_accepted = 202,
        http_status_code_non_authoritative_information = 203,
        http_status_code_no_content = 204,
        http_status_code_reset_content = 205,
        http_status_code_partial_content = 206,
        http_status_code_multiple_choices = 300,
        http_status_code_moved_permanently = 301,
        http_status_code_found = 302,
        http_status_code_see_other = 303,
        http_status_code_not_modified = 304,
        http_status_code_use_proxy = 305,
        http_status_code_unused = 306,
        http_status_code_temporary_redirect = 307,
        http_status_code_permanent_redirect = 308,
        http_status_code_bad_request = 400,
        http_status_code_unauthorized = 401,
        http_status_code_payment_required = 402,
        http_status_code_forbidden = 403,
        http_status_code_not_found = 404,
        http_status_code_method_not_allowed = 405,
        http_status_code_not_acceptable = 406,
        http_status_code_proxy_authentication_required = 407,
        http_status_code_request_timeout = 408,
        http_status_code_conflict = 409,
        http_status_code_gone = 410,
        http_status_code_length_required = 411,
        http_status_code_precondition_failed = 412,
        http_status_code_payload_too_large = 413,
        http_status_code_uri_too_long = 414,
        http_status_code_unsupported_media_type = 415,
        http_status_code_range_not_satisfiable = 416,
        http_status_code_expectation_failed = 417,
        http_status_code_misdirected_request = 421,
        http_status_code_unprocessable_entity = 422,
        http_status_code_locked = 423,
        http_status_code_failed_dependency = 424,
        http_status_code_upgrade_required = 426,
        http_status_code_precondition_required = 428,
        http_status_code_too_many_requests = 429,
        http_status_code_request_header_fields_too_large = 431,
        http_status_code_internal_server_error = 500,
        http_status_code_not_implemented = 501,
        http_status_code_bad_gateway = 502,
        http_status_code_service_unavailable = 503,
        http_status_code_gateway_timeout = 504,
        http_status_code_http_version_not_supported = 505,
        http_status_code_variant_also_negotiates = 506,
        http_status_code_insufficient_storage = 507,
        http_status_code_loop_detected = 508,
        http_status_code_not_extended = 510,
        http_status_code_network_authentication_required = 511
    };

	struct http_request_info
	{
		http_headers headers;
		http_method method;
		std::string path;
		uint64_t contentLength;
	};

    using http_response_callback = std::function<void(const void *data,size_t size)>;

    std::string http_method_to_string(http_method m);
    http_method string_to_http_method(const std::string &str);
    std::string get_http_content_type(const std::string &filePath);
}

#endif