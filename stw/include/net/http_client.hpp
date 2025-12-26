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

#include <string>
#include <cstdint>
#include <unordered_map>
#include <functional>

namespace stw
{
	namespace curl
	{
		bool load_library();
	}

	class http_client_request
	{
    public:
        http_client_request();
        ~http_client_request();
        void set_url(const std::string &url);
        std::string get_url() const;
        void set_content(void *data, uint64_t size, bool copyData = false);
        uint8_t *get_content() const;
        uint64_t get_content_length() const;
        void set_content_type(const std::string &contentType);
        std::string get_content_type() const;
        void set_header(const std::string &key, const std::string &value);
        std::unordered_map<std::string,std::string> get_headers() const;
    private:
        std::string url;
        uint8_t *content;        
        uint64_t contentLength;
        std::string contentType;
        std::unordered_map<std::string,std::string> header;
        bool ownsData;
	};

	class http_client_response
	{
    friend class http_client;
    public:
        http_client_response();
        int get_status_code() const;
        uint64_t get_content_length() const;
        std::unordered_map<std::string,std::string> &get_headers();
    private:
        int32_t statusCode;
        uint64_t contentLength;
        std::unordered_map<std::string,std::string> header;
	};
	
	using http_client_response_callback = std::function<void(const void *data,size_t size)>;

	class http_client
	{
	public:
		http_client_response_callback onResponse;
		http_client();
		bool get(const http_client_request &req, http_client_response &res);
		bool post(const http_client_request &req, http_client_response &res);
		void set_validate_certificate(bool validate);
		bool validate_certificate() const;
	private:
		bool validateCertificate;
		static bool parse_header(const std::string &responseText, std::unordered_map<std::string,std::string> &header, int &statusCode, uint64_t &contentLength);
		static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp);
		static size_t header_callback(void *contents, size_t size, size_t nmemb, void *userp);
	};
}

#endif