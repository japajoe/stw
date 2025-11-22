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
		~http_client();
		bool get(const http_request &req, http_response &res);
        bool post(const http_request &req, http_response &res);
        void set_validate_certificate(bool validate);
        bool validate_certificate() const;
	private:
        bool validateCertificate;
        static bool parse_header(const std::string &responseText, http_headers &header, int &statusCode, uint64_t &contentLength);
        static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);
        static size_t header_callback(void* contents, size_t size, size_t nmemb, void* userp);
	};
}

#endif