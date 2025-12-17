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

#include "stw_web_socket.hpp"
#include "stw_string.hpp"
#include <cstring>
#include <sstream>
#include <algorithm>
#ifdef _WIN32
#undef min  // to remove any existing macro definition on windows
#endif
#include <atomic>
#include <regex>
#include <random>
#include <iostream>

namespace stw
{
    static void write_error(const std::string &message) 
	{
		std::cerr << message << '\n';
    }

    static bool is_little_endian() 
	{
        uint16_t num = 1;
        uint8_t* ptr = reinterpret_cast<uint8_t*>(&num);
        return ptr[0] == 1; // If the least significant byte is 1, it's little-endian
    }

    static void swap_bytes(void *buffer, size_t size) 
	{
        if(size < 2)
            return;
        
        uint8_t *bytes = reinterpret_cast<uint8_t*>(buffer);

        for (size_t i = 0; i < size / 2; ++i) 
		{
            uint8_t temp = bytes[i];
            bytes[i] = bytes[size - i - 1];
            bytes[size - i - 1] = temp;
        }
    }

    static void network_to_host_order(void *src, void *dest, size_t size) 
	{
        if(is_little_endian())
            swap_bytes(src, size);
        std::memcpy(dest, src, size);
    }

    static void host_to_network_order(void *src, void *dest, size_t size) 
	{
        if(is_little_endian())
            swap_bytes(src, size);
        std::memcpy(dest, src, size);
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

    static std::string generate_key() 
	{
        uint8_t randomBytes[16];

        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<int> distribution(0, 255);

        for (auto& byte : randomBytes) 
		{
            byte = static_cast<unsigned char>(distribution(generator));
        }

        return crypto::base64_encode(randomBytes, 16);
    }

    static std::string generate_accept_key(const std::string &websocketKey) 
	{
        const std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        std::string acceptKey = websocketKey + guid;

		constexpr size_t SHA_DIGEST_LENGTH = 20;
        uint8_t hash[SHA_DIGEST_LENGTH];
        crypto::create_sha1_hash(reinterpret_cast<const uint8_t*>(acceptKey.c_str()), acceptKey.size(), hash);

        return crypto::base64_encode(hash, SHA_DIGEST_LENGTH);
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
    }

 	static bool resolve(const std::string &uri, std::string &ip, uint16_t &port, std::string &hostname, bool ignoreIPv6) 
	{
        std::string scheme, host, path;

        if(!uri_get_scheme(uri, scheme)) 
		{
            write_error("Failed to get scheme from URI");
            return false;
        }

        if(!uri_get_host(uri, host)) 
		{
            write_error("Failed to get host from URI");
            return false;
        }


        if(!uri_get_path(uri, path)) 
		{
            write_error("Failed to get path from URI");
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
            if(scheme == "wss")
                port = 443;
            else if(scheme == "ws") 
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
			std::string error = "getaddrinfo error: " + std::string(gai_strerror(status));
            write_error(error);
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
			{ 
                // IPv6
                if(ignoreIPv6)
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

	web_socket::web_socket()
	{
        connectionState = web_socket_connection_state_disconnected;
	}

    web_socket::web_socket(const std::string &certificatePath, const std::string &privateKeyPath)
    {
        connectionState = web_socket_connection_state_disconnected;
		
		if(!sslContext.create(certificatePath, privateKeyPath))
			throw std::runtime_error("Failed to create ssl context");
		if(sslInstance.create(&sslContext))
			throw std::runtime_error("Failed to create ssl");
    }

	web_socket::~web_socket()
	{
		close();
	}

	bool web_socket::connect(const std::string &url)
	{
		if(sock.get_file_descriptor() >= 0)
			return false;
		
		std::string URL = url;

        if(!string::ends_with(URL, "/"))
            URL += "/";

        std::string scheme, hostName, path;

        if(!uri_get_scheme(URL, scheme)) 
		{
            write_error("web_socket::connect: failed to determine scheme from URI " + URL);
            return false;
        }

        if(!uri_get_path(URL, path)) 
		{
            write_error("web_socket::connect: failed to determine path from URI " + URL);
            return false;
        }

        std::string ip;
        uint16_t port;
        
        if(!resolve(URL, ip, port, hostName, true)) 
        {
            write_error("web_socket::connect: failed to resolve IP from URI " + URL);
            return false;
        }

        const int noDelayFlag = 1;
		sock.set_option(IPPROTO_TCP, TCP_NODELAY, &noDelayFlag, sizeof(int));

        if(!sock.connect(ip, port)) 
		{
            write_error("web_socket::connect: failed to connect");
            close();
            return false;
        }

        bool secure = scheme == "wss" ? true : false;

        if(secure)
        {
			sslContext.create();
            sslInstance.create(&sslContext);
            
            sslInstance.set_file_descriptor(sock.get_file_descriptor());
            sslInstance.set_hostname(hostName);

			if(!sslInstance.connect())
            {
                write_error("web_socket::connect: failed to SSL connect");
                close();
                return false;
            }
        }

        std::string webKey = generate_key();
        std::string request;

        request += "GET " + path + " HTTP/1.1\r\n";
        request += "Host: " + hostName + "\r\n";
        request += "Upgrade: websocket\r\n";
        request += "Connection: Upgrade\r\n";
        request += "Sec-WebSocket-Key: " + webKey + "\r\n";
        request += "Sec-WebSocket-Version: 13\r\n\r\n";

        if(write(request.c_str(), request.size()) <= 0) 
		{
            write_error("web_socket::connect: failed to send 'Connection-Upgrade' request");
            close();
            return false;
        }

		std::string responsesHeaderText;

        // Read the response header
        if(read_header(responsesHeaderText) != http_header_error_none)
        {
			write_error("web_socket::connect: failed to read response header");
            close();
            return false;
        }

		http_headers responseHeaders;
		int statusCode = 0;
		uint64_t contentLength = 0;

        // Parse the response header
        if(!parse_response_header(responsesHeaderText, responseHeaders, statusCode, contentLength))
        {
			write_error("web_socket::connect: failed to parse response header");
            close();
            return false;
        }

		if(!responseHeaders.contains("sec-websocket-accept"))
		{
			write_error("web_socket::connect: sec-websocket-accept header not found");
            close();
            return false;
		}

        const std::string &acceptKey = responseHeaders["sec-websocket-accept"];

        if(!verify_key(acceptKey, webKey)) 
		{
            write_error("web_socket::connect: handshake keys mismatch");
            close();
            return false;
        }

        connectionState = web_socket_connection_state_connected;

        return true;
	}

    bool web_socket::bind(const std::string &bindAddress, uint16_t port) 
    {
		if(sock.bind(bindAddress, port))
		{
			int noDelayFlag = 1;
			set_option(IPPROTO_TCP, TCP_NODELAY, (char *)&noDelayFlag, sizeof(int));
			return true;
		}

		return false;
    }

    bool web_socket::listen(int32_t backlog)
    {
        if(sock.listen(backlog))
		{
            connectionState = web_socket_connection_state_connected;
			return true;
		}

        return false;
    }

    bool web_socket::accept(web_socket &client)
    {
        if(sock.get_file_descriptor() < 0) 
        {
            write_error("web_socket::accept: failed to accept because socket isn't initialized");
            return false;
        }

        if(client.sock.get_file_descriptor() >= 0) 
        {
            write_error("web_socket::accept: failed to accept client because its socket is already initialized");
            return false;
        }

		if(!sock.accept(&client.sock))
			return false;

        if(sslContext.is_valid()) 
        {
            client.sslInstance.create(&sslContext);
            
            if(!client.sslInstance.is_valid()) 
            {
                write_error("web_socket::accept: failed to create client SSL");
                client.close();
                return false;
            }

			client.sslInstance.set_file_descriptor(client.sock.get_file_descriptor());

			if(!client.sslInstance.accept())
            {
                write_error("WebSocket::accept: failed to set client SSL file descriptor");
				client.sslInstance.destroy();
                client.close();
                return false;
            }
        }

        auto sendBadRequest = [] (web_socket &s) -> void {
            std::string response = "HTTP/1.1 400\r\n\r\n";
            s.write_all_bytes(response.c_str(), response.size());
            s.close();
        };

        std::string requestHeaderText;
        if(read_header(requestHeaderText) != http_header_error_none)
        {
			write_error("web_socket::accept: failed to read request header");
            sendBadRequest(client);
            return false;
        }

		http_headers requestHeaders;
		std::string method;
        std::string path;
		uint64_t contentLength = 0;

        // Parse the request header
        if(!parse_request_header(requestHeaderText, requestHeaders, method, path, contentLength))
        {
			write_error("web_socket::accept: failed to parse request header");
            sendBadRequest(client);
            return false;
        }

        if(method != "get") 
        {
            write_error("web_socket::accept: invalid HTTP method");
            sendBadRequest(client);
            return false;
        }

        const std::vector<std::string> requiredHeaders = {
            "upgrade", "connection", "sec-websocket-version", "sec-websocket-key"
        };

        for (const auto &key : requiredHeaders) 
        {
            if (!requestHeaders.contains(key)) 
            {
                write_error("web_socket::accept: missing required header field: " + key);
                sendBadRequest(client);
                return false;
            }
        }

        std::string upgrade = requestHeaders["upgrade"];
        std::string connection = requestHeaders["connection"];
        std::string version = requestHeaders["sec-websocket-version"];
        std::string webKey = requestHeaders["sec-websocket-key"];

        if(upgrade != "websocket") 
        {
            write_error("web_socket::accept: failed to find 'websocket'");
            sendBadRequest(client);
            return false;
        }

        if(!string::contains(connection, "Upgrade")) 
        {
            write_error("web_socket::accept: failed to find 'Upgrade'");
            sendBadRequest(client);
            return false;
        }

        if(version != "13") 
        {
            write_error("web_socket::accept: version mismatch");
            sendBadRequest(client);
            return false;
        }

        std::string acceptKey = generate_accept_key(webKey);

        std::string response = "HTTP/1.1 101 Switching Protocols\r\n";
        response += "Upgrade: websocket\r\n";
        response += "Connection: Upgrade\r\n";
        response += "Server: Testing\r\n";
        response += "Sec-WebSocket-Accept: " + acceptKey + "\r\n\r\n";

        if(!client.write_all_bytes(response.data(), response.size()))
        {
            write_error("web_socket::accept: failed to send handshake response");
            client.close();
            return false;
        }

        client.connectionState = web_socket_connection_state_connected;

        return true;
    }

	void web_socket::close()
	{
        sock.close();
        sslInstance.destroy();
        sslContext.destroy();
        connectionState = web_socket_connection_state_disconnected;
	}

	web_socket_result web_socket::send(web_socket_opcode opcode, const void *data, uint64_t size, bool masked)
	{
		bool first = true;

        if(data != nullptr && size == 0) 
		{
            return web_socket_result_invalid_argument;
        }

        web_socket_result status = web_socket_result_ok;

        if(data != nullptr) 
		{
            const uint64_t chunkSize = 1024;
            uint64_t totalSize = size;
            const uint8_t *pPayload = reinterpret_cast<const uint8_t*>(data);

            while (totalSize > 0) 
			{
                uint64_t length = std::min(totalSize, chunkSize);
                web_socket_opcode opc = first ? opcode : web_socket_opcode_continuation;
                bool fin = totalSize - length == 0;
                
                status = write_frame(opc, fin, pPayload, length, masked);
                
                if(status != web_socket_result_ok)
                    return status;

                pPayload += length;
                totalSize -= length;
                first = false;
            }
        } 
		else 
		{
            return write_frame(opcode, true, nullptr, 0, masked);
        }

        return status;
	}

	web_socket_result web_socket::receive()
	{
		web_socket_message message;

        bool messageComplete = false;

        auto isControlFrameOpcode = [] (uint8_t opcode) -> bool {
            // Check if the opcode is one of the control frame opcodes
            return (opcode == 0x8 || opcode == 0x9 || opcode == 0xA);
        };

        //Loop and collect frames until we get to the 'fin' frame or run into an error
        while(!messageComplete) 
		{
            uint16_t peekData = 0;
            int64_t peekedBytes = peek(&peekData, 2); //Header is at least 2 bytes

            //If there is no data to read we can return early
            if(peekedBytes <= 0)
                return web_socket_result_ok;

            web_socket_frame frame = {0};

            web_socket_result result = read_frame(&frame);
            
            if(result != web_socket_result_ok) 
			{
                switch(result) 
				{
                    case web_socket_result_connection_error:
                    case web_socket_result_control_frame_too_big:
                        return drop_connection(result);
                    default:
                        return result;
                }
            }

            web_socket_opcode opcode = static_cast<web_socket_opcode>(frame.opcode);

            switch(opcode) 
			{
                case web_socket_opcode_text:
                case web_socket_opcode_binary: 
				{
                    message.opcode = opcode;

                    if(message.payload.size() == 0 && frame.payloadLength > 0) 
					{
                        message.payload.insert(message.payload.end(), frame.payload.begin(), frame.payload.end());
                    } 
					else 
					{
                        //This shouldn't happen
                        return drop_connection(web_socket_result_no_data);
                    }

                    break;
                }
                case web_socket_opcode_continuation: 
				{
                    if(message.payload.size() > 0 && frame.payloadLength > 0) 
					{
                        message.payload.insert(message.payload.end(), frame.payload.begin(), frame.payload.end());
                    } 
					else 
					{
                        //This shouldn't happen
                        return drop_connection(web_socket_result_no_data);
                    }

                    break;
                }
                case web_socket_opcode_close: 
				{
					//Close frames might contain a 2 byte status code
					//In case of receiving a status code, we must echo it with the response
                    if(connectionState != web_socket_connection_state_disconnecting) 
                    {
                        connectionState = web_socket_connection_state_disconnecting;

                        if(frame.payload.size() >= 2) 
                        {
                            write_frame(web_socket_opcode_close, true, &frame.payload[0], 2, !frame.masked);
                        } 
                        else 
                        {
                            write_frame(web_socket_opcode_close, true, nullptr, 0, !frame.masked);
                        }
                    }
                    break;
                }
                case web_socket_opcode_ping: 
				{
                    //If frame was masked, it means a client sent it
                    //Only servers are supposed to send ping messages
                    //Clients need to respond with a pong
                    if(!frame.masked)
                        write_frame(web_socket_opcode_pong, true, nullptr, 0, true);
                    break;
                }
                case web_socket_opcode_pong: 
				{
                    break;
                }
                default: 
				{
                    return web_socket_result_invalid_op_code;
                }
            }

            if(frame.fin) 
			{
                if(isControlFrameOpcode(frame.opcode)) 
				{
                    web_socket_message m;
                    m.opcode = static_cast<web_socket_opcode>(frame.opcode);

                    if(frame.payloadLength > 0) 
                        m.payload.insert(m.payload.end(), frame.payload.begin(), frame.payload.end());

                    if(onReceived)
                        onReceived(this, m);

                    //Even if this is a fin frame, we must check if we expect more data
                    //If the message payload size is greater than 0, we haven't received its fin frame yet
                    if(message.payload.size() == 0) 
					{
                        messageComplete = true;
                        break;
                    }
                } 
				else 
				{
                    //According to RFC 6455 we need to verify if text opcodes contain valid UTF-8
                    if(message.opcode == web_socket_opcode_text) 
					{
                        if(!string::is_valid_utf8(&message.payload[0], message.payload.size())) 
                            return drop_connection(web_socket_result_utf8_error);
                    }

                    if(onReceived)
                        onReceived(this, message);
                    messageComplete = true;
                    break;
                }
            }
        }

        return web_socket_result_ok;
	}

	web_socket_result web_socket::drop_connection(web_socket_result result)
	{
        close();
        return result;
	}

	web_socket_result web_socket::read_frame(web_socket_frame *frame) 
	{
        uint8_t header[32] = {0};

        if(!read_all_bytes(header, 2))
            return web_socket_result_connection_error;

        frame->fin = (header[0] & 0x80) != 0;
        frame->opcode = header[0] & 0x0F;
        frame->masked = (header[1] & 0x80) != 0;
        uint8_t payloadLength = header[1] & 0x7F;
        frame->payloadLength = static_cast<uint64_t>(payloadLength);

        auto isControlFrameOpcode = [] (uint8_t opcode) -> bool {
            // Check if the opcode is one of the control frame opcodes
            return (opcode == 0x8 || opcode == 0x9 || opcode == 0xA);
        };

        if (payloadLength == 126) 
		{
            uint8_t extendedLength[2] = {0};
            
            if(!read_all_bytes(extendedLength, 2))
                return web_socket_result_connection_error;

            uint16_t sizeHostOrder = 0;
            network_to_host_order(extendedLength, &sizeHostOrder, sizeof(uint16_t));
            frame->payloadLength = static_cast<uint64_t>(sizeHostOrder);
        } 
		else if (payloadLength == 127) 
		{
            uint8_t extendedLength[8] = {0};
            
            if(!read_all_bytes(extendedLength, 8))
                return web_socket_result_connection_error;

            network_to_host_order(extendedLength, &frame->payloadLength, sizeof(uint64_t));
        }

        if(isControlFrameOpcode(frame->opcode) && (frame->payloadLength > 125 || !frame->fin)) 
            return web_socket_result_control_frame_too_big;

        if(frame->masked) 
		{
            if(!read_all_bytes(frame->mask, 4))
                return web_socket_result_connection_error;
        }

        if(frame->payloadLength > 0) 
		{
            frame->payload.resize(frame->payloadLength);
            
            if(!read_all_bytes(&frame->payload[0], frame->payloadLength)) 
			{
                frame->payloadLength = 0;
                return web_socket_result_connection_error;
            }

            if(frame->masked) 
			{
                for(size_t i = 0; i < frame->payloadLength; i++)
				{
                    frame->payload[i] = frame->payload[i] ^ frame->mask[i % 4];
				}
            }
        }
		
        return web_socket_result_ok;
    }

	web_socket_result web_socket::write_frame(web_socket_opcode opcode, bool fin, const void *payload, uint64_t payloadSize, bool applyMask)
	{
        uint8_t frame[32] = {0};

        size_t offset = 0;

        // 1st byte: FIN, RSV1, RSV2, RSV3, Opcode
        uint8_t firstByte = 0;
        if (fin) 
            firstByte |= 0x80;  // FIN = 1 if true
            
        firstByte |= (static_cast<uint8_t>(opcode) & 0x0F);  // Opcode (lower 4 bits)
        frame[offset++] = firstByte;

        // 2nd byte: Mask bit and Payload Length (7 bits)
        uint8_t secondByte = 0;
        if (applyMask) 
            secondByte |= 0x80;  // Mask bit set if true
            
        if (payloadSize <= 125) 
		{
            secondByte |= static_cast<uint8_t>(payloadSize);  // Payload length (7 bits)
            frame[offset++] = secondByte;
        } 
		else if (payloadSize > 125 && payloadSize <= 65535) 
		{
            secondByte |= 126;  // Special case for lengths > 125 but <= 65535
            frame[offset++] = secondByte;

            uint16_t sizeNetworkOrder = static_cast<uint16_t>(payloadSize);
            host_to_network_order(&sizeNetworkOrder, &frame[offset], sizeof(uint16_t));
            offset += sizeof(uint16_t);
        } 
		else 
		{
            secondByte |= 127;  // Special case for lengths > 65535
            frame[offset++] = secondByte;

            host_to_network_order(&payloadSize, &frame[offset], sizeof(uint64_t));
            offset += sizeof(uint64_t);
        }

        if(applyMask) 
		{
            std::random_device rd;
            std::mt19937 generator(rd());
            std::uniform_int_distribution<int> distribution(0, 255);

            uint8_t mask[4] = {0};

            for (size_t i = 0; i < 4; ++i) 
			{
                mask[i] = static_cast<unsigned char>(distribution(generator));
                frame[offset++] = mask[i];
            }

            uint8_t* payloadBytes = (uint8_t*)payload;

            for (uint64_t i = 0; i < payloadSize; ++i) 
			{
                payloadBytes[i] = payloadBytes[i] ^ mask[i % 4];
            }
        }

        if(!write_all_bytes(frame, offset))
            return web_socket_result_connection_error;
        
        if(payloadSize == 0)
            return  web_socket_result_ok;

        if(!write_all_bytes(payload, payloadSize))
            return web_socket_result_connection_error;

        return web_socket_result_ok;
    }

	bool web_socket::set_option(int level, int option, const void *value, uint32_t valueSize)
	{
        return sock.set_option(level, option, value, valueSize);
	}

	bool web_socket::set_blocking(bool isBlocking)
	{
        return sock.set_blocking(isBlocking);
	}

    void web_socket::set_validate_certificate(bool validate)
    {
        if(sslContext.is_valid())
        {
			sslContext.set_validate_peer(validate);
        }
    }

    web_socket_connection_state web_socket::get_state() const
    {
        return connectionState;
    }

	int64_t web_socket::read(void *buffer, size_t size)
	{
        int64_t n = 0;
        if(sslInstance.is_valid())
            n = sslInstance.read(buffer, size);
        else
			n = sock.read(buffer, size);
        return n;
	}

	int64_t web_socket::peek(void *buffer, size_t size)
	{
        int64_t n = 0;
        if(sslInstance.is_valid())
            n = sslInstance.peek(buffer, size);
        else
            n = sock.peek(buffer, size);
        return n;
	}

	int64_t web_socket::write(const void *buffer, size_t size)
	{
        int64_t n = 0;
        if(sslInstance.is_valid())
            n = sslInstance.write(buffer, size);
        else
            n = sock.write(buffer, size);
        return n;
	}

    bool web_socket::read_all_bytes(void *buffer, size_t size)
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

    bool web_socket::write_all_bytes(const void *buffer, size_t size)
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

    http_header_error web_socket::read_header(std::string &header)
    {
        const size_t maxHeaderSize = 16384;
        const size_t bufferSize = maxHeaderSize;
        std::vector<char> buffer;
        buffer.resize(bufferSize);
        int64_t headerEnd = 0;
        int64_t totalHeaderSize = 0;
        bool endFound = false;

        auto find_header_end = [] (const char* haystack, const char* needle, size_t haystackLength, size_t needleLength) -> int64_t {
            for (size_t i = 0; i <= haystackLength - needleLength; ++i) {
                if (memcmp(haystack + i, needle, needleLength) == 0) {
                    return static_cast<int>(i); // Found the needle, return the index
                }
            }
            return -1; // Not found
        };

        char *pBuffer = buffer.data();

        // Peek to find the end of the header
        while (true) 
        {
            int64_t bytesPeeked = peek(pBuffer, bufferSize);

            totalHeaderSize += bytesPeeked;

            if(totalHeaderSize > maxHeaderSize) 
            {
                printf("header_error_max_size_exceeded 1, %zu/%zu\n", totalHeaderSize, maxHeaderSize);
                return http_header_error_max_size_exceeded;
            }

            if (bytesPeeked < 0)
                return http_header_error_failed_to_peek;

            //Don't loop indefinitely...
            if(bytesPeeked == 0)
                break;
            
            // Look for the end of the header (double CRLF)
            int64_t end = find_header_end(pBuffer, "\r\n\r\n", bytesPeeked, 4);

            if(end >= 0)
            {
                headerEnd = end + 4; //Include the length of the CRLF
                endFound = true;
                break;                
            }
        }

        if(!endFound)
            return http_header_error_end_not_found;

        // Now read the header
        header.resize(headerEnd);
        int64_t bytesRead = read(header.data(), headerEnd);
        if (bytesRead < 0) 
            return http_header_error_failed_to_read;

        if(header.size() > maxHeaderSize) 
        {
            printf("header_error_max_size_exceeded 2, %zu/%zu\n", header.size(), maxHeaderSize);
            return http_header_error_max_size_exceeded;
        }

        return http_header_error_none;
    }

    bool web_socket::parse_response_header(const std::string &responseText, http_headers &header, int &statusCode, uint64_t &contentLength)
    {
        std::istringstream responseStream(responseText);
        std::string line;
        size_t count = 0;

        while(std::getline(responseStream, line))
        {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

            if(line.size() == 0)
                continue;

            if(count == 0)
            {
                if(line[line.size() - 1] == ' ')
                    line.pop_back();

                auto parts = string::split(line, ' ', 0);
                
                if(parts.size() < 2)
                    return false;

                if(!string::try_parse_int32(parts[1], statusCode))
                    return false;
            }
            else
            {
                auto parts = string::split(line, ':', 2);

                if(parts.size() == 2)
                {
                    parts[0] = string::to_lower(parts[0]);
                    parts[1] = string::trim_start(parts[1]);
                    header[parts[0]] = parts[1];
                }
            }

            count++;
        }

        if(header.contains("content-length"))
        {
            if(!string::try_parse_uint64(header["content-length"], contentLength))
                contentLength = 0;
        }

        return count > 0;
    }

    bool web_socket::parse_request_header(const std::string &responseText, http_headers &header, std::string &method, std::string &path, uint64_t &contentLength)
    {
        std::istringstream responseStream(responseText);
        std::string line;
        size_t count = 0;

        while(std::getline(responseStream, line))
        {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

            if(line.size() == 0)
                continue;

            if(count == 0)
            {
                if(line[line.size() - 1] == ' ')
                    line.pop_back();

                auto parts = string::split(line, ' ', 0);
                
                if(parts.size() < 2)
                    return false;

                method = string::to_lower(parts[0]);
                path = parts[1];
            }
            else
            {
                auto parts = string::split(line, ':', 2);

                if(parts.size() == 2)
                {
                    parts[0] = string::to_lower(parts[0]);
                    parts[1] = string::trim_start(parts[1]);
                    header[parts[0]] = parts[1];
                }
            }

            count++;
        }

        if(header.contains("content-length"))
        {
            if(!string::try_parse_uint64(header["content-length"], contentLength))
                contentLength = 0;
        }

        return count > 0;
    }

    bool web_socket::verify_key(const std::string& receivedAcceptKey, const std::string& originalKey) 
	{
        std::string expectedAcceptKey = generate_accept_key(originalKey);
        return receivedAcceptKey == expectedAcceptKey;
    }

	web_socket_message::web_socket_message() 
	{
        opcode = web_socket_opcode_continuation;
    }

    web_socket_message::web_socket_message(const web_socket_message &other) noexcept 
	{
        opcode = other.opcode;
        payload = other.payload;
    }

    web_socket_message::web_socket_message(web_socket_message &&other) 
	{
        opcode = other.opcode;
        payload = std::move(other.payload);
    }

    web_socket_message& web_socket_message::operator=(const web_socket_message &other) 
	{
        if(this != &other) 
		{
            opcode = other.opcode;
            payload = other.payload;
        }
        return *this;
    }

    web_socket_message& web_socket_message::operator=(web_socket_message &&other) noexcept 
	{
        if(this != &other) 
		{
            opcode = other.opcode;
            payload = std::move(other.payload);
        }
        return *this;
    }

    bool web_socket_message::get_text(std::string &s) 
	{
        if(payload.size() == 0)
            return false;

        char *ptr = reinterpret_cast<char*>(payload.data());

        s = std::string(ptr, payload.size());

        return true;
    }

    bool web_socket_message::get_raw(std::vector<uint8_t> &data) 
	{
        if(payload.size() == 0)
            return false;

        data.insert(data.end(), payload.begin(), payload.end());
        return true;
    }
}