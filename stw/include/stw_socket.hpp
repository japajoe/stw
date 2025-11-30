#ifndef STW_SOCKET_HPP
#define STW_SOCKET_HPP

#ifdef _WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0600
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#endif
#include <cstdint>
#include <string>

namespace stw
{
    enum address_family 
	{
        address_family_af_inet = AF_INET,
        address_family_af_inet6 = AF_INET6
    };

    enum ip_version
	{
        ip_version_ip_v4,
        ip_version_ip_v6,
        ip_version_invalid
    };

    typedef struct sockaddr_in sockaddr_in_t;
    typedef struct sockaddr_in6 sockaddr_in6_t;

    typedef union 
	{
        sockaddr_in_t ipv4;
        sockaddr_in6_t ipv6;
    } socket_address_t;

    typedef struct 
	{
        int32_t fd;
        socket_address_t address;
        address_family addressFamily;
    } socket_t;

	enum socket_protocol_type
	{
		socket_protocol_type_tcp,
		socket_protocol_type_udp
	};

	class socket
	{
	public:
		socket();
		socket(socket_protocol_type protocolType);
		socket(socket_protocol_type protocolType, const std::string &ip, uint16_t port);
		socket(const socket &other);
		socket(socket &&other) noexcept;
		socket &operator=(const socket &other);
		socket &operator=(socket &&other) noexcept;
		~socket();
		bool connect(const std::string &ip, uint16_t port);
		bool bind(const std::string &bindAddress, uint16_t port);
		bool listen(int32_t backlog);
		bool accept(socket *target);
		void close();
		int64_t read(void *buffer, size_t size);
		int64_t peek(void *buffer, size_t size);
		int64_t write(const void *buffer, size_t size);
		bool read_all(void *buffer, size_t size);
		bool write_all(const void *buffer, size_t size);
		bool set_option(int32_t level, int32_t option, const void *value, uint32_t valueSize);
		bool set_timeout(uint32_t seconds);
		bool set_blocking(bool block);
		int32_t get_file_descriptor() const;
		std::string get_ip() const;
		static bool resolve(const std::string &uri, std::string &ip, uint16_t &port, std::string &hostname, bool forceIPv4);
	private:
		socket_t s;
		socket_protocol_type protocolType;
	};
}

#endif