#ifndef STW_SSL_HPP
#define STW_SSL_HPP

#include <string>
#include <cstdint>

namespace stw
{
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