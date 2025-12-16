#include "web_socket_app.hpp"
#include <iostream>
#include <thread>

web_socket_app::web_socket_app()
{
	stw::signal::register_handler([this](int n){on_signal(n);});
	stw::signal::register_signal(SIGINT);
	stw::signal::register_signal(SIGTERM);
#ifndef STW_PLATFORM_WINDOWS
	stw::signal::register_signal(SIGPIPE);
#endif

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
}

void web_socket_app::run()
{
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

void web_socket_app::on_signal(int n)
{
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
}