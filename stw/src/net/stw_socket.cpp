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

#include "stw_socket.hpp"
#include "../system/stw_string.hpp"
#include <cstring>
#include <sstream>
#include <atomic>
#include <regex>
#include <exception>
#include <iostream>

namespace stw
{
	static bool uri_get_scheme(const std::string &uri, std::string &value) 
	{
        std::regex schemeRegex(R"(([^:/?#]+):\/\/)");
        std::smatch match;
        if (std::regex_search(uri, match, schemeRegex)) 
		{
            value = match[1];
            return true;
        }
        return false;
    }

    static bool uri_get_host(const std::string &uri, std::string &value) 
	{
        std::regex hostRegex(R"(:\/\/([^/?#]+))");
        std::smatch match;
        if (std::regex_search(uri, match, hostRegex)) 
		{
            value = match[1];
            return true;
        }
        return false;
    }

    static bool uri_get_path(const std::string &uri, std::string &value) 
	{
        std::regex pathRegex(R"(:\/\/[^/?#]+([^?#]*))");
        std::smatch match;
        if (std::regex_search(uri, match, pathRegex)) 
		{
            value = match[1];
            return true;
        }
        return false;
    }

    static ip_version detect_ip_version(const std::string &ip)
    {
        struct sockaddr_in sa;
        struct sockaddr_in6 sa6;

        if (inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1) 
            return ip_version_ip_v4;

        if (inet_pton(AF_INET6, ip.c_str(), &(sa6.sin6_addr)) == 1)
            return ip_version_ip_v6;

        return ip_version_invalid;
    };

	class socket_exception : public std::exception 
	{
	public:
		socket_exception(const std::string& message) : message_(message) {}

		const char* what() const noexcept override 
		{
			return message_.c_str();
		}
	private:
		std::string message_;
	};

	static std::atomic<int32_t> gSocketCount = 0;

    static void load_winsock()
    {
    #if defined(STW_PLATFORM_WINDOWS)
		if(gSocketCount.load() == 0)
		{
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
            {
                throw socket_exception("Failed to initialize winsock");
            }
		}
    #endif

		gSocketCount.store(gSocketCount.load() + 1);        
    }

    static void unload_winsock()
    {
    #if defined(STW_PLATFORM_WINDOWS)
		if(gSocketCount.load() == 1)
		{
			WSACleanup();
		}
    #endif

		gSocketCount.store(gSocketCount.load() - 1);
    }

    socket::socket()
    {
        this->protocolType = socket_protocol_type_tcp;

		std::memset(&s, 0, sizeof(socket_t));
		s.fd = -1;
        s.addressFamily = address_family_af_inet;

		load_winsock();
    }

	socket::socket(socket_protocol_type protocolType)
	{
        this->protocolType = protocolType;

		std::memset(&s, 0, sizeof(socket_t));
		s.fd = -1;
        s.addressFamily = address_family_af_inet;

		load_winsock();
	}

    socket::socket(socket_protocol_type protocolType, const std::string &ip, uint16_t port)
    {
        this->protocolType = protocolType;

		std::memset(&s, 0, sizeof(socket_t));
		s.fd = -1;
        s.addressFamily = address_family_af_inet;

        load_winsock();

		ip_version ipVersion = detect_ip_version(ip);

        if(ipVersion == ip_version_invalid) 
            throw socket_exception("Failed to detect IP version");
        
        s.addressFamily = (ipVersion == ip_version_ip_v4) ? address_family_af_inet : address_family_af_inet6;

        s.fd = ::socket(static_cast<int>(s.addressFamily), protocolType == socket_protocol_type_tcp ? SOCK_STREAM : SOCK_DGRAM, 0);

        if(s.fd < 0) 
            throw socket_exception("Failed to create socket");

        if(ipVersion == ip_version_ip_v4) 
		{
			std::memset(&s.address.ipv4, 0, sizeof(s.address.ipv4));
            s.address.ipv4.sin_family = AF_INET;
            s.address.ipv4.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &s.address.ipv4.sin_addr);
        } 
		else 
		{
			std::memset(&s.address.ipv6, 0, sizeof(s.address.ipv6));
            s.address.ipv6.sin6_family = AF_INET6;
            s.address.ipv6.sin6_port = htons(port);
            inet_pton(AF_INET6, ip.c_str(), &s.address.ipv6.sin6_addr);
        }
    }

	socket::socket(socket &&other) noexcept
	{
		std::memcpy(&s, &other.s, sizeof(socket_t));
		other.s.fd = -1;
		protocolType = other.protocolType;
	}

	socket &socket::operator=(socket &&other) noexcept
	{
		if(this != &other)
		{
			std::memcpy(&s, &other.s, sizeof(socket_t));
			other.s.fd = -1;
			protocolType = other.protocolType;
		}
		return *this;
	}

	socket::~socket()
	{
		close();

        unload_winsock();
	}

	bool socket::connect(const std::string &ip, uint16_t port)
	{
		if(s.fd >= 0)
			return false;

		ip_version ipVersion = detect_ip_version(ip);

        if(ipVersion == ip_version_invalid) 
            return false;
        
        address_family addressFamily = (ipVersion == ip_version_ip_v4) ? address_family_af_inet : address_family_af_inet6;

        s.fd = ::socket(static_cast<int>(addressFamily), protocolType == socket_protocol_type_tcp ? SOCK_STREAM : SOCK_DGRAM, 0);

        if(s.fd < 0) 
            return false;

        s.addressFamily = addressFamily;

        int32_t connectionResult = 0;

        if(ipVersion == ip_version_ip_v4) 
		{
			std::memset(&s.address.ipv4, 0, sizeof(s.address.ipv4));
            s.address.ipv4.sin_family = AF_INET;
            s.address.ipv4.sin_port = htons(port);
            inet_pton(AF_INET, ip.c_str(), &s.address.ipv4.sin_addr);
            connectionResult = ::connect(s.fd, (struct sockaddr*)&s.address.ipv4, sizeof(s.address.ipv4));
        } 
		else 
		{
			std::memset(&s.address.ipv6, 0, sizeof(s.address.ipv6));
            s.address.ipv6.sin6_family = AF_INET6;
            s.address.ipv6.sin6_port = htons(port);
            inet_pton(AF_INET6, ip.c_str(), &s.address.ipv6.sin6_addr);
            connectionResult = ::connect(s.fd, (struct sockaddr*)&s.address.ipv6, sizeof(s.address.ipv6));
        }

        if(connectionResult < 0) 
		{
			std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
            close();
            return false;
        }

		return true;
	}

    bool socket::bind(const std::string &bindAddress, uint16_t port)
    {
        if(s.fd < 0) 
        {
            int32_t newfd = ::socket(static_cast<int>(s.addressFamily), protocolType == socket_protocol_type_tcp ? SOCK_STREAM : SOCK_DGRAM, 0);

            if(newfd < 0) 
            {
                std::cerr << "socket::bind: " << strerror(errno) << std::endl;
                return false;
            }
            s.fd = newfd;
        }

        if(s.addressFamily == address_family_af_inet) 
        {
            sockaddr_in address = {0};
            address.sin_family = AF_INET;
            address.sin_port = htons(port);
            address.sin_addr.s_addr = INADDR_ANY;

            if (inet_pton(AF_INET, bindAddress.c_str(), &address.sin_addr) <= 0) 
                return false;

            std::memcpy(&s.address.ipv4, &address, sizeof(sockaddr_in));

            int reuseFlag = 1;
            set_option(SOL_SOCKET, SO_REUSEADDR, &reuseFlag, sizeof(int));

            return ::bind(s.fd, (struct sockaddr*)&address, sizeof(address)) == 0;
        } 
        else if (s.addressFamily == address_family_af_inet6) 
        {
            sockaddr_in6 address = {0};
            address.sin6_family = AF_INET6;
            address.sin6_port = htons(port);
            address.sin6_addr = in6addr_any;

            if (inet_pton(AF_INET6, bindAddress.c_str(), &address.sin6_addr) <= 0) 
                return false;

            std::memcpy(&s.address.ipv6, &address, sizeof(sockaddr_in6));

            int reuse = 1;
            set_option(SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

            return ::bind(s.fd, (struct sockaddr*)&address, sizeof(address)) == 0;
        }

        return false;
    }

	bool socket::listen(int32_t backlog)
	{
        if(s.fd < 0) 
            return false;

        int32_t result = ::listen(s.fd, backlog);

        return result == 0;
	}

#if defined(STW_PLATFORM_WINDOWS)
    const char* get_error_message(int errorCode) {
        switch (errorCode) {
            case WSAEWOULDBLOCK:
                return "Resource temporarily unavailable (non-blocking mode)";
            case WSAEFAULT:
                return "The address specified is invalid";
            case WSAENOTSOCK:
                return "The socket operation encountered a problem";
            case WSAEINTR:
                return "A blocking Windows Sockets 1.1 operation was interrupted";
            // Add more cases as needed
            default:
                return "An unknown error occurred";
        }
    }
#endif

	bool socket::accept(socket *target)
	{
		if(!target)
			return false;

        if(s.fd < 0) 
            return false;

        if(target->s.fd >= 0) 
            return false;

        struct sockaddr_storage address;
        std::memset(&address, 0, sizeof(address));
        uint32_t addressLength = sizeof(sockaddr_storage);
        
    #if defined(STW_PLATFORM_WINDOWS)
        target->s.fd = ::accept(s.fd,  (struct sockaddr*)&address, (int32_t*)&addressLength);
    #elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
        target->s.fd = ::accept(s.fd,  (struct sockaddr*)&address, &addressLength);
    #elif
        return false;
    #endif
        
        if(target->s.fd < 0)
            return false;

        if(address.ss_family == AF_INET) 
        {
            sockaddr_in_t *pAddress = (sockaddr_in_t*)&address;
            std::memcpy(&target->s.address.ipv4, pAddress, sizeof(sockaddr_in_t));
            target->s.addressFamily = address_family_af_inet;
        } 
        else if(address.ss_family == AF_INET6) 
        {
            sockaddr_in6_t *pAddress = (sockaddr_in6_t*)&address;
            std::memcpy(&target->s.address.ipv6, pAddress, sizeof(sockaddr_in6_t));
            target->s.addressFamily = address_family_af_inet6;
        } 
        else 
        {
            target->close();
            return false;
        }

        target->protocolType = protocolType;

		return true;
	}

	void socket::close()
	{
		if(s.fd >= 0) 
		{
            auto emptyBuffers = [this] () {
                std::vector<uint8_t> buffer(1024);
                while(true) 
                {
                    int64_t n = read(buffer.data(), buffer.size());
                    if(n <= 0)
                        break;
                }
            };


        #if defined(STW_PLATFORM_WINDOWS)
            ::shutdown(s.fd, SD_SEND);
            emptyBuffers();
            closesocket(s.fd);
        #elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
            ::shutdown(s.fd, SHUT_WR);
            emptyBuffers();
            ::close(s.fd);
        #endif
            s.fd = -1;
        }
	}

	int64_t socket::read(void *buffer, size_t size)
	{
		int64_t n = 0;
	#if defined(STW_PLATFORM_WINDOWS)
		n = ::recv(s.fd, (char*)buffer, size, 0);
	#elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		n = ::recv(s.fd, buffer, size, 0);
	#endif
		return n;
	}

	int64_t socket::peek(void *buffer, size_t size)
	{
		int64_t n = 0;
	#if defined(STW_PLATFORM_WINDOWS)
		n = ::recv(s.fd, (char*)buffer, size, MSG_PEEK);
	#elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		n = ::recv(s.fd, buffer, size, MSG_PEEK);
	#endif
		return n;
	}

	int64_t socket::write(const void *buffer, size_t size)
	{
		int64_t n = 0;
	#if defined(STW_PLATFORM_WINDOWS)
		n = ::send(s.fd, (char*)buffer, size, 0);
	#elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		n = ::send(s.fd, buffer, size, 0);
	#endif
		return n;
	}

	bool socket::read_all(void *buffer, size_t size)
	{
		uint8_t *ptr = static_cast<uint8_t*>(buffer);
        size_t totalRead = 0;

        while (totalRead < size) 
		{
            int64_t bytesRead = read(ptr + totalRead, size - totalRead);
            
            if (bytesRead < 0) 
			{
                // An error occurred
                return false;
            } 
			else if (bytesRead == 0) 
			{
                // Connection closed
                return false;
            }

            totalRead += bytesRead;
        }

        return true; // All bytes read successfully
	}

	bool socket::write_all(const void *buffer, size_t size)
	{
		const uint8_t *ptr = static_cast<const uint8_t*>(buffer);
        size_t totalSent = 0;

        while (totalSent < size) 
		{
            int64_t bytesSent = write(ptr + totalSent, size - totalSent);
            
            if (bytesSent < 0) 
			{
                // An error occurred
                return false;
            } 
			else if (bytesSent == 0) 
			{
                // Connection closed
                return false;
            }

            totalSent += bytesSent;
        }

        return true; // All bytes sent successfully
	}

	bool socket::set_option(int32_t level, int32_t option, const void *value, uint32_t valueSize)
	{
    #if defined(STW_PLATFORM_WINDOWS)
        return setsockopt(s.fd, level, option, (char*)value, valueSize) != 0 ? false : true;
    #elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
        return setsockopt(s.fd, level, option, value, valueSize) != 0 ? false : true;
    #else
        return false;
    #endif
	}

    bool socket::set_timeout(uint32_t seconds)
    {
    #if defined(STW_PLATFORM_WINDOWS)
        DWORD timeout = seconds * 1000; // Convert to milliseconds
        return setsockopt(s.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) != 0 ? false : true;
    #elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
        struct timeval timeout;
        timeout.tv_sec = seconds;
        timeout.tv_usec = 0;
        return setsockopt(s.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) != 0 ? false : true;
    #else
        return false;
    #endif
    }

    bool socket::set_blocking(bool block)
    {
    #if defined(STW_PLATFORM_WINDOWS)
        u_long mode = block ? 0 : 1; // 1 to enable non-blocking socket
        if (ioctlsocket(s.fd, FIONBIO, &mode) != 0)
            return false;
        return true;
    #elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
        int flags = fcntl(s.fd, F_GETFL, 0);
        if (flags == -1) return -1; // Error handling

        if (block)
            flags &= ~O_NONBLOCK;    // Clear the non-blocking flag
        else
            flags |= O_NONBLOCK;     // Set the non-blocking flag

        return fcntl(s.fd, F_SETFL, flags) >= 0;
    #else
        return false;
    #endif
    }

    bool socket::set_no_delay(bool noDelay)
    {
        const int32_t noDelayFlag = noDelay ? 1 : 0;
        return set_option(IPPROTO_TCP, TCP_NODELAY, &noDelayFlag, sizeof(int32_t));
    }

	int32_t socket::get_file_descriptor() const
	{
		return s.fd;
	}

    std::string socket::get_ip() const
    {
        if(s.fd < 0)
        {
            if(s.addressFamily == address_family_af_inet)
                return "0.0.0.0";
            else
                return "::";
        }

        std::string result;

        if(s.addressFamily == address_family_af_inet)
        {
            result.resize(INET_ADDRSTRLEN);
            if(inet_ntop(AF_INET, &s.address.ipv4.sin_addr, result.data(), INET_ADDRSTRLEN) != nullptr)
                return result;
            return "0.0.0.0";
        }
        else
        {
            result.resize(INET6_ADDRSTRLEN);
            if(inet_ntop(AF_INET6, &s.address.ipv6.sin6_addr, result.data(), INET6_ADDRSTRLEN) != nullptr)
                return result;
            return "::";
        }
    }

    // ToDo: Handle IPv6 in a better way
	bool socket::resolve(const std::string &uri, std::string &ip, uint16_t &port, std::string &hostname, bool forceIPv4)
	{
		std::string scheme, host, path;

        if(!uri_get_scheme(uri, scheme)) 
		{
            std::cerr << "Failed to get scheme from URI\n";
            return false;
        }

        if(!uri_get_host(uri, host)) 
		{
            std::cerr << "Failed to get host from URI\n";
            return false;
        }


        if(!uri_get_path(uri, path)) 
		{
            std::cerr << "Failed to get path from URI\n";
            return false;
        }        

        if(string::contains(host, ":")) 
		{
            auto parts = string::split(host, ':');

            if(parts.size() != 2)
                return false;
            
            //Get rid of the :port part in the host
            host = parts[0];

            if(!string::try_parse_uint16(parts[1], port))
                return false;
        } 
		else 
		{
            if(scheme == "https")
                port = 443;
            else if(scheme == "http") 
                port = 80;
			else 
                return false;
        }

        // Resolve the hostname to an IP address
        struct addrinfo hints, *res;
        std::memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP

        int status = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);

        if (status != 0) 
		{
			std::string error = "getaddrinfo error: " + std::string(gai_strerror(status)) + "\n";
            std::cerr << error;
            return false;
        }

        hostname = host;

        for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) 
		{
            void* addr;

            // Get the pointer to the address itself
            if (p->ai_family == AF_INET) 
			{ // IPv4
                struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
                addr = &(ipv4->sin_addr);
            } 
			else 
			{ // IPv6
                if(forceIPv4)
                    continue;
                struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)p->ai_addr;
                addr = &(ipv6->sin6_addr);
            }

            // Convert the IP to a string
            char ipstr[INET6_ADDRSTRLEN];
            inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
            ip = ipstr;
        }

        freeaddrinfo(res);
        return true;
	}
}