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

#ifndef SUGAR_WEB_SOCKET_HPP
#define SUGAR_WEB_SOCKET_HPP

#include "socket.hpp"
#include "http.hpp"
#include "ssl.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

namespace stw
{
    enum web_socket_opcode
	{
		web_socket_opcode_continuation = 0x0,
		web_socket_opcode_text = 0x1,
		web_socket_opcode_binary = 0x2,
		web_socket_opcode_reserved1 = 0x3,
		web_socket_opcode_reserved2 = 0x4,
		web_socket_opcode_reserved3 = 0x5,
		web_socket_opcode_reserved4 = 0x6,
		web_socket_opcode_reserved5 = 0x7,
		web_socket_opcode_close = 0x8,
		web_socket_opcode_ping = 0x9,
		web_socket_opcode_pong = 0xA,
		web_socket_opcode_reserved6 = 0xB,
		web_socket_opcode_reserved7 = 0xC,
		web_socket_opcode_reserved8 = 0xD,
		web_socket_opcode_reserved9 = 0xE,
		web_socket_opcode_reserved10 = 0xF
    };

    enum web_socket_result 
	{
		web_socket_result_ok = 0,
		web_socket_result_no_data = 1,
		web_socket_result_connection_error = 2,
		web_socket_result_allocation_error = 3,
		web_socket_result_utf8_error = 4,
		web_socket_result_invalid_op_code = 5,
		web_socket_result_invalid_argument = 6,
		web_socket_result_control_frame_too_big = 7
    };

    enum web_socket_connection_state
	{
        web_socket_connection_state_connected,
        web_socket_connection_state_disconnected,
        web_socket_connection_state_disconnecting
    };

    class web_socket_message 
	{
    public:
        web_socket_opcode opcode;
        std::vector<uint8_t> payload;
        web_socket_message();
        web_socket_message(const web_socket_message &other) noexcept;
        web_socket_message(web_socket_message &&other);
        web_socket_message& operator=(const web_socket_message &other);
        web_socket_message& operator=(web_socket_message &&other) noexcept;
        bool get_text(std::string &s);
        bool get_raw(std::vector<uint8_t> &data);
    };

    struct web_socket_frame 
	{
        uint8_t fin;
        uint8_t RSV1;
        uint8_t RSV2;
        uint8_t RSV3;
        uint8_t opcode;
        bool masked;
        uint8_t mask[4];
        uint64_t payloadLength;
        std::vector<uint8_t> payload;
    };

	class web_socket;
	using web_socket_received_callback = std::function<void(const web_socket *socket, web_socket_message &message)>;

	class web_socket
	{
	public:
		web_socket_received_callback onReceived;
		web_socket();
		web_socket(const std::string &certificatePath, const std::string &privateKeyPath);
		~web_socket();
		bool connect(const std::string &url);
		bool bind(const std::string &bindAddress, uint16_t port);
		bool listen(int32_t backlog);
		bool accept(web_socket &client);
		void close();
		web_socket_result send(web_socket_opcode opcode, const void *data, uint64_t size, bool masked);
		web_socket_result receive();
        bool set_option(int level, int option, const void *value, uint32_t valueSize);
        bool set_blocking(bool isBlocking);
		void set_validate_certificate(bool validate);
		web_socket_connection_state get_state() const;
	private:
		socket sock;
		ssl sslInstance;
		ssl_context sslContext;
		web_socket_connection_state connectionState;
		web_socket_result drop_connection(web_socket_result result);
		web_socket_result read_frame(web_socket_frame *frame);
		web_socket_result write_frame(web_socket_opcode opcode, bool fin, const void *payload, uint64_t payloadSize, bool applyMask);
		int64_t read(void *buffer, size_t size);
		int64_t peek(void *buffer, size_t size);
		int64_t write(const void *buffer, size_t size);
		bool read_all_bytes(void *buffer, size_t size);
		bool write_all_bytes(const void *buffer, size_t size);
		http_header_error read_header(std::string &header);
		static bool parse_response_header(const std::string &responseText, http_headers &header, int &statusCode, uint64_t &contentLength);
		static bool parse_request_header(const std::string &responseText, http_headers &header, std::string &method, std::string &path, uint64_t &contentLength);
		static bool verify_key(const std::string& receivedAcceptKey, const std::string& originalKey);
	};
}

#endif