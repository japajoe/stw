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

#ifndef STW_SSL_HPP
#define STW_SSL_HPP

#include <string>
#include <cstdint>

namespace stw
{
	namespace openssl
	{
		bool load_library();
	}

	typedef void SSL;
	typedef void SSL_CTX;

	class ssl_context
	{
	friend class ssl;
	public:
		ssl_context();
		~ssl_context();
		ssl_context(const ssl_context &other) = delete;
		ssl_context(ssl_context &&other) noexcept;
		ssl_context operator=(const ssl_context &other) = delete;
		ssl_context &operator=(ssl_context &&other) noexcept;
		bool create(const std::string &certificateFilePath, const std::string &privateKeyFilePath);
		bool create();
		void destroy();
		bool is_valid() const;
		void set_validate_peer(bool validate);
	private:
		SSL_CTX *pContext;
	};

	class ssl
	{
	public:
		ssl();
		~ssl();
		ssl(const ssl &other) = delete;
		ssl(ssl &&other) noexcept;
		ssl operator=(const ssl &other) = delete;
		ssl &operator=(ssl &&other) noexcept;
		bool create(ssl_context *context);
		void destroy();
		bool set_file_descriptor(int32_t fd);
		bool set_hostname(const std::string &hostName);
		bool connect();
		bool accept();
		int64_t read(void *buffer, size_t size);
		int64_t write(const void *buffer, size_t size);
		int64_t peek(void *buffer, size_t size);
		bool read_all(void *buffer, size_t size);
		bool write_all(const void *buffer, size_t size);
		bool is_valid() const;
	private:
		SSL *pSsl;
	};

	class crypto
	{
	public:
		static std::string base64_encode(const uint8_t* buffer, size_t size);
		static uint8_t *create_sha1_hash(const uint8_t *d, size_t n, uint8_t *md);
	};
}

#endif