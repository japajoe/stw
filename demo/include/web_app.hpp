#ifndef WEB_APP_HPP
#define WEB_APP_HPP

#include "stw.hpp"

class web_app
{
public:
	web_app();
	void run();
private:
	stw::http_server server;
	stw::http_server_configuration config;
	stw::file_cache fileCache;
	void on_signal(int n);
	void on_request(stw::http_connection *connection, const stw::http_request_info &request);
};

#endif