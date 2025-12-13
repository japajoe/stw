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
	stw::http_router router;
	void on_request(stw::http_connection *connection, const stw::http_request_info &request);
	void send_file_content(stw::http_connection *connection, uint32_t statusCode, const std::string &filePath);
};

#endif