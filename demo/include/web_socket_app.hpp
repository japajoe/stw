#ifndef WEB_SOCKET_APP_HPP
#define WEB_SOCKET_APP_HPP

#include "stw.hpp"

class web_socket_app
{
public:
	web_socket_app();
	void run();
private:
	stw::web_socket socket;
	void on_signal(int n);
};

#endif