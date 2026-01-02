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

#include "socket.hpp"
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <vector>
#include <atomic>
#include <regex>
#include <exception>
#include <iostream>

namespace stw
{
    static bool string_contains(const std::string &haystack, const std::string &needle) 
	{
        return haystack.find(needle) != std::string::npos;
    }

	static std::vector<std::string> string_split(const std::string& str, char separator, size_t maxParts = 0) 
    {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end = 0;

        while ((end = str.find(separator, start)) != std::string::npos) 
        {
            result.push_back(str.substr(start, end - start));
            start = end + 1;

            if (maxParts > 0 && result.size() >= maxParts - 1) 
                break; // Stop if we have reached maximum parts
        }
        result.push_back(str.substr(start)); // Add the last part
        return result;
    }

	static bool string_try_parse_uint16(const std::string &value, uint16_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

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

#if defined(STW_SOCKET_PLATFORM_WINDOWS)
	static std::atomic<int32_t> gSocketCount = 0;

    static void load_winsock()
    {
		if(gSocketCount.load() == 0)
		{
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
                throw socket_exception("Failed to initialize winsock");
		}
		gSocketCount.store(gSocketCount.load() + 1);        
    }

    static void unload_winsock()
    {
		if(gSocketCount.load() == 1)
		{
			WSACleanup();
		}
		gSocketCount.store(gSocketCount.load() - 1);
    }
#endif

    socket::socket()
    {
        this->protocolType = socket_protocol_type_tcp;

		std::memset(&s, 0, sizeof(socket_t));
		s.fd = INVALID_SOCKET_HANDLE;
        s.addressFamily = address_family_af_inet;

	#if defined(STW_SOCKET_PLATFORM_WINDOWS)
		load_winsock();
	#endif
    }

	socket::socket(socket_protocol_type protocolType)
	{
        this->protocolType = protocolType;

		std::memset(&s, 0, sizeof(socket_t));
		s.fd = INVALID_SOCKET_HANDLE;
        s.addressFamily = address_family_af_inet;

	#if defined(STW_SOCKET_PLATFORM_WINDOWS)
		load_winsock();
	#endif
	}

    socket::socket(socket_protocol_type protocolType, const std::string &ip, uint16_t port)
    {
        this->protocolType = protocolType;

		std::memset(&s, 0, sizeof(socket_t));
		s.fd = INVALID_SOCKET_HANDLE;
        s.addressFamily = address_family_af_inet;

	#if defined(STW_SOCKET_PLATFORM_WINDOWS)
		load_winsock();
	#endif

		ip_version ipVersion = detect_ip_version(ip);

        if(ipVersion == ip_version_invalid) 
            throw socket_exception("Failed to detect IP version");
        
        s.addressFamily = (ipVersion == ip_version_ip_v4) ? address_family_af_inet : address_family_af_inet6;

        s.fd = ::socket(static_cast<int>(s.addressFamily), protocolType == socket_protocol_type_tcp ? SOCK_STREAM : SOCK_DGRAM, 0);

        if(s.fd == INVALID_SOCKET_HANDLE) 
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
		other.s.fd = INVALID_SOCKET_HANDLE;
		protocolType = other.protocolType;
	}

	socket &socket::operator=(socket &&other) noexcept
	{
		if(this != &other)
		{
			std::memcpy(&s, &other.s, sizeof(socket_t));
			other.s.fd = INVALID_SOCKET_HANDLE;
			protocolType = other.protocolType;
		}
		return *this;
	}

	socket::~socket()
	{
		close();
	#if defined(STW_SOCKET_PLATFORM_WINDOWS)
		unload_winsock();
	#endif
	}

	bool socket::connect(const std::string &ip, uint16_t port)
	{
		if(s.fd >= INVALID_SOCKET_HANDLE)
			return false;

		ip_version ipVersion = detect_ip_version(ip);

        if(ipVersion == ip_version_invalid) 
            return false;
        
        address_family addressFamily = (ipVersion == ip_version_ip_v4) ? address_family_af_inet : address_family_af_inet6;

        s.fd = ::socket(static_cast<int>(addressFamily), protocolType == socket_protocol_type_tcp ? SOCK_STREAM : SOCK_DGRAM, 0);

        if(s.fd == INVALID_SOCKET_HANDLE) 
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
			std::cout << "Socket creation failed: " << strerror(errno) << std::endl;
            close();
            return false;
        }

		return true;
	}

    bool socket::bind(const std::string &bindAddress, uint16_t port)
    {
        if(s.fd == INVALID_SOCKET_HANDLE) 
        {
            int32_t newfd = ::socket(static_cast<int>(s.addressFamily), protocolType == socket_protocol_type_tcp ? SOCK_STREAM : SOCK_DGRAM, 0);

            if(newfd == INVALID_SOCKET_HANDLE) 
            {
                std::cout << "socket::bind: " << strerror(errno) << std::endl;
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
        if(s.fd == INVALID_SOCKET_HANDLE) 
            return false;

        int32_t result = ::listen(s.fd, backlog);

        return result == 0;
	}

#if defined(STW_SOCKET_PLATFORM_WINDOWS)
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

        if(s.fd == INVALID_SOCKET_HANDLE) 
            return false;

        if(target->s.fd != INVALID_SOCKET_HANDLE) 
            return false;

        struct sockaddr_storage address;
        std::memset(&address, 0, sizeof(address));
		stw_socklen_t addressLength = sizeof(sockaddr_storage);
        
    #if defined(STW_SOCKET_PLATFORM_WINDOWS)
        target->s.fd = ::accept(s.fd,  (struct sockaddr*)&address, &addressLength);
    #elif defined(STW_SOCKET_PLATFORM_UNIX)
        target->s.fd = ::accept(s.fd,  (struct sockaddr*)&address, &addressLength);
    #elif
        return false;
    #endif
        
        if(target->s.fd == INVALID_SOCKET_HANDLE)
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
		if(s.fd != INVALID_SOCKET_HANDLE) 
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


        #if defined(STW_SOCKET_PLATFORM_WINDOWS)
            ::shutdown(s.fd, SD_SEND);
            emptyBuffers();
            closesocket(s.fd);
        #elif defined(STW_SOCKET_PLATFORM_UNIX)
            ::shutdown(s.fd, SHUT_WR);
            emptyBuffers();
            ::close(s.fd);
        #endif
            s.fd = INVALID_SOCKET_HANDLE;
        }
	}

	int64_t socket::read(void *buffer, size_t size)
	{
		int64_t n = 0;
	#if defined(STW_SOCKET_PLATFORM_WINDOWS)
		n = ::recv(s.fd, (char*)buffer, size, 0);
	#elif defined(STW_SOCKET_PLATFORM_UNIX)
		n = ::recv(s.fd, buffer, size, 0);
	#endif
		return n;
	}

	int64_t socket::peek(void *buffer, size_t size)
	{
		int64_t n = 0;
	#if defined(STW_SOCKET_PLATFORM_WINDOWS)
		n = ::recv(s.fd, (char*)buffer, size, MSG_PEEK);
	#elif defined(STW_SOCKET_PLATFORM_UNIX)
		n = ::recv(s.fd, buffer, size, MSG_PEEK);
	#endif
		return n;
	}

	int64_t socket::write(const void *buffer, size_t size)
	{
		int64_t n = 0;
	#if defined(STW_SOCKET_PLATFORM_WINDOWS)
		n = ::send(s.fd, (char*)buffer, size, 0);
	#elif defined(STW_SOCKET_PLATFORM_UNIX)
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
    #if defined(STW_SOCKET_PLATFORM_WINDOWS)
        return setsockopt(s.fd, level, option, (char*)value, valueSize) != 0 ? false : true;
    #elif defined(STW_SOCKET_PLATFORM_UNIX)
        return setsockopt(s.fd, level, option, value, valueSize) != 0 ? false : true;
    #else
        return false;
    #endif
	}

    bool socket::set_timeout(uint32_t seconds)
    {
    #if defined(STW_SOCKET_PLATFORM_WINDOWS)
        DWORD timeout = seconds * 1000; // Convert to milliseconds
        return setsockopt(s.fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) != 0 ? false : true;
    #elif defined(STW_SOCKET_PLATFORM_UNIX)
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
    #if defined(STW_SOCKET_PLATFORM_WINDOWS)
        u_long mode = block ? 0 : 1; // 1 to enable non-blocking socket
        if (ioctlsocket(s.fd, FIONBIO, &mode) != 0)
            return false;
        return true;
    #elif defined(STW_SOCKET_PLATFORM_UNIX)
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

	socket_handle socket::get_file_descriptor() const
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
            std::cout << "Failed to get scheme from URI\n";
            return false;
        }

        if(!uri_get_host(uri, host)) 
		{
            std::cout << "Failed to get host from URI\n";
            return false;
        }


        if(!uri_get_path(uri, path)) 
		{
            std::cout << "Failed to get path from URI\n";
            return false;
        }        

        if(string_contains(host, ":")) 
		{
            auto parts = string_split(host, ':');

            if(parts.size() != 2)
                return false;
            
            //Get rid of the :port part in the host
            host = parts[0];

            if(!string_try_parse_uint16(parts[1], port))
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
            std::cout << error;
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