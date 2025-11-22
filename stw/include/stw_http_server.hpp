#ifndef STW_HTTP_SERVER
#define STW_HTTP_SERVER

#include "stw_socket.hpp"
#include "stw_ssl.hpp"
#include "stw_http.hpp"
#include "stw_stream.hpp"
#include <string>
#include <functional>
#include <vector>
#include <atomic>

namespace stw
{
	class http_connection
	{
	public:
		http_connection(socket &connection, ssl &s);
		http_connection(const http_connection &other);
		http_connection(http_connection &&other) noexcept;
		http_connection &operator=(const http_connection &other);
		http_connection &operator=(http_connection &&other) noexcept;
		int64_t read(void *buffer, size_t size);
		int64_t write(const void *buffer, size_t size);
		int64_t peek(void *buffer, size_t size);
		int64_t read_all(void *buffer, size_t size);
		int64_t write_all(const void *buffer, size_t size);
		bool write_response(uint32_t statusCode);
		bool write_response(uint32_t statusCode, const http_headers *headers);
		bool write_response(uint32_t statusCode, const http_headers *headers, const void *content, uint64_t contentLength, const std::string &contentType);
		bool write_response(uint32_t statusCode, const http_headers *headers, stream *content, const std::string &contentType);
		void close();
		bool is_secure() const;
	private:
		socket connection;
		ssl s;
	};

	struct http_request_info
	{
		http_headers headers;
		http_method method;
		std::string path;
		uint64_t contentLength;
	};

    struct http_server_configuration 
	{
        uint16_t port;
        uint16_t portHttps;
        uint32_t maxHeaderSize;
        std::string bindAddress;
        std::string certificatePath;
        std::string privateKeyPath;
		std::string publicHtmlPath;
		std::string privateHtmlPath;
        std::string hostName;
        bool useHttps;
        bool useHttpsForwarding;
        void load_default();
		bool load_from_file(const std::string &filePath);
    };

	using http_request_handler = std::function<void(http_connection *connection, const http_request_info &request)>;

	class http_server
	{
	public:
		http_request_handler onRequest;
		http_server();
		~http_server();
		void run(http_server_configuration &config);
		void stop();
	private:
		std::vector<socket> listeners;
		ssl_context sslContext;
		http_server_configuration config;
		std::atomic<bool> quit;
		void on_request(http_connection connection);
		http_header_error read_header(http_connection *connection, std::string &header);
		bool parse_request_header(const std::string &responseText, http_headers &header, std::string &method, std::string &path, uint64_t &contentLength);
	};
}

#endif