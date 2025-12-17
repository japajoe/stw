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

#ifndef STW_SOCKET_HPP
#define STW_SOCKET_HPP

#include "stw_platform.hpp"

#if defined(STW_PLATFORM_WINDOWS)
	#ifdef _WIN32_WINNT
	#undef _WIN32_WINNT
	#endif
	#define _WIN32_WINNT 0x0600
	#include <winsock2.h>
	#include <ws2tcpip.h>
#endif

#if defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
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
		socket(const socket &other) = delete;
		socket(socket &&other) noexcept;
		socket &operator=(const socket &other) = delete;
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
		bool set_no_delay(bool noDelay);
		int32_t get_file_descriptor() const;
		std::string get_ip() const;
		static bool resolve(const std::string &uri, std::string &ip, uint16_t &port, std::string &hostname, bool forceIPv4);
	private:
		socket_t s;
		socket_protocol_type protocolType;
	};
}

#endif