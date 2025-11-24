#include "stw_socket.hpp"
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

	static bool try_parse_uint16(const std::string &value, uint16_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

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

	socket::socket(socket_protocol_type protocolType)
	{
        this->protocolType = protocolType;

		std::memset(&s, 0, sizeof(socket_t));
		s.fd = -1;
        s.addressFamily = address_family_af_inet;

		if(gSocketCount.load() == 0)
		{
		#ifdef _WIN32
			if(gSocketCount.load() == 0)
			{
				WSADATA wsaData;
				if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
				{
					throw socket_exception("Failed to initialize winsock");
				}
			}
		#endif
		}

		gSocketCount.store(gSocketCount.load() + 1);
	}

	socket::socket(const socket &other)
	{
		s = other.s;
		protocolType = other.protocolType;
	}

	socket::socket(socket &&other) noexcept
	{
		std::memcpy(&s, &other.s, sizeof(socket_t));
		other.s.fd = -1;
		protocolType = other.protocolType;
	}

	socket &socket::operator=(const socket &other)
	{
		if(this != &other)
		{
			s = other.s;
			protocolType = other.protocolType;
		}
		return *this;
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

		if(gSocketCount.load() == 1)
		{
		#ifdef _WIN32
			WSACleanup();
		#endif
		}

		gSocketCount.store(gSocketCount.load() - 1);
	}

	bool socket::connect(const std::string &ip, uint16_t port)
	{
		if(s.fd >= 0)
			return false;

		auto detect_ip_version = [] (const std::string &ip) -> ip_version {
			struct sockaddr_in sa;
			struct sockaddr_in6 sa6;

			if (inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) == 1) 
				return ip_version_ip_v4;

			if (inet_pton(AF_INET6, ip.c_str(), &(sa6.sin6_addr)) == 1)
				return ip_version_ip_v6;

			return ip_version_invalid;
		};

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
                std::cerr << "Socket::bind: " << strerror(errno) << std::endl;
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

#ifdef _WIN32
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

        struct sockaddr addr;
        std::memset(&addr, 0, sizeof(addr));
        uint32_t size = sizeof(addr);
        uint32_t addrLen = sizeof(addr);
        
    #ifdef _WIN32
        target->s.fd = ::accept(s.fd,  &addr, (int32_t*)&size);
    #else
        target->s.fd = ::accept(s.fd,  &addr, &size);
    #endif
        
        if(target->s.fd < 0)
            return false;
        
        if(size == sizeof(sockaddr_in_t)) 
        {
            std::memcpy(&target->s.address.ipv4, &addr, size);
            target->s.addressFamily = address_family_af_inet;
        } 
        else if(size == sizeof(sockaddr_in6_t)) 
        {
			const int sss = sizeof(target->s.address.ipv6);
			const int ssss = sizeof(addr);
            std::memcpy(&target->s.address.ipv6, &addr, size);
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
        #ifdef _WIN32
            ::shutdown(s.fd, SD_SEND);
            closesocket(s.fd);
        #else
            ::shutdown(s.fd, SHUT_WR);
            ::close(s.fd);
        #endif
            s.fd = -1;
        }
	}

	int64_t socket::read(void *buffer, size_t size)
	{
		int64_t n = 0;
	#ifdef _WIN32
		n = ::recv(s.fd, (char*)buffer, size, 0);
	#else
		n = ::recv(s.fd, buffer, size, 0);
	#endif
		return n;
	}

	int64_t socket::peek(void *buffer, size_t size)
	{
		int64_t n = 0;
	#ifdef _WIN32
		n = ::recv(s.fd, (char*)buffer, size, MSG_PEEK);
	#else
		n = ::recv(s.fd, buffer, size, MSG_PEEK);
	#endif
		return n;
	}

	int64_t socket::write(const void *buffer, size_t size)
	{
		int64_t n = 0;
	#ifdef _WIN32
		n = ::send(s.fd, (char*)buffer, size, 0);
	#else
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
    #ifdef _WIN32
        return setsockopt(s.fd, level, option, (char*)value, valueSize) != 0 ? false : true;
    #else
        return setsockopt(s.fd, level, option, value, valueSize) != 0 ? false : true;
    #endif
	}

    bool socket::set_blocking(bool block)
    {
    #ifdef _WIN32
        u_long mode = block ? 0 : 1; // 1 to enable non-blocking socket
        if (ioctlsocket(s.fd, FIONBIO, &mode) != 0)
            return false;
        return true;
    #else
        int flags = fcntl(s.fd, F_GETFL, 0);
        if (flags == -1) return -1; // Error handling

        if (block)
            flags &= ~O_NONBLOCK;    // Clear the non-blocking flag
        else
            flags |= O_NONBLOCK;     // Set the non-blocking flag

        return fcntl(s.fd, F_SETFL, flags) >= 0;
    #endif
    }

	int32_t socket::get_file_descriptor() const
	{
		return s.fd;
	}

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

        if(string_contains(host, ":")) 
		{
            auto parts = string_split(host, ':');

            if(parts.size() != 2)
                return false;
            
            //Get rid of the :port part in the host
            host = parts[0];

            if(!try_parse_uint16(parts[1], port))
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