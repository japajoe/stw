#include "web_app.hpp"
#include <iostream>

web_app::web_app()
{
	stw::signal::register_handler([this](int n){on_signal(n);});
	stw::signal::register_signal(SIGINT);
#ifndef _WIN32
	stw::signal::register_signal(SIGPIPE);
#endif

	server.onRequest = [this] (stw::http_connection *c, const stw::http_request_info &r) {
		on_request(c, r);
	};
}

void web_app::run()
{
	if(!config.load_from_file("config.ini"))
		return;

	server.run(config);
}

void web_app::on_signal(int n)
{
	if(n == SIGINT)
		server.stop();
}

void web_app::on_request(stw::http_connection *connection, const stw::http_request_info &request)
{
	std::cout << "[" << stw::http_method_to_string(request.method) << "]: " << request.path << "\n";

	std::string path = config.publicHtmlPath + request.path;
	
	if(stw::file::exists(path))
	{
		if(stw::file::is_within_directory(path, config.publicHtmlPath))
		{
			stw::file_stream fileStream(path, stw::file_access_read);
			std::string contentType = stw::get_http_content_type(path);
			connection->write_response(200, nullptr, &fileStream, contentType);
			return;
		}
	}

	stw::file_stream notFoundStream(config.privateHtmlPath + "/404.html", stw::file_access_read);
	connection->write_response(404, nullptr, &notFoundStream, "text/html");
}