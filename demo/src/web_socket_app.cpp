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

#include "web_socket_app.hpp"
#include <iostream>
#include <thread>

web_socket_app::web_socket_app()
{
	stw::signal::register_handler([this](int n) { 
		if(n == SIGINT)
		{
			socket.send(stw::web_socket_opcode_close, nullptr, 0, true);
			socket.close();
			close(0);
		}
		if(n == SIGTERM)
		{
			socket.send(stw::web_socket_opcode_close, nullptr, 0, true);
			socket.close();
		}
	});

	stw::signal::register_signal(SIGINT);
	stw::signal::register_signal(SIGTERM);
#ifndef STW_PLATFORM_WINDOWS
	stw::signal::register_signal(SIGPIPE);
#endif
}

void web_socket_app::run()
{
	socket.onReceived = [] (const stw::web_socket *s, stw::web_socket_message &m) {
		if(m.opcode == stw::web_socket_opcode_text)
		{
			std::string text;
			
			if(m.get_text(text))
			{
				std::cout << text << "\n\n";
			}
		}
	};

	if(socket.connect("wss://pumpportal.fun/api/data"))
	{
		socket.set_blocking(false);
		std::string request = R"({ "method": "subscribeNewToken" })";
		socket.send(stw::web_socket_opcode_text, request.c_str(), request.size(), true);

		while(socket.get_state() == stw::web_socket_connection_state_connected)
		{
			socket.receive();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}