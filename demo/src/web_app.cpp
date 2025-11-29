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
#ifndef _WIN32
	if(n == SIGPIPE)
		std::cerr << "Broken pipe\n";
#endif
}

void web_app::on_request(stw::http_connection *connection, const stw::http_request_info &request)
{
	std::cout << "[" << stw::http_method_to_string(request.method) << "]: " << request.path << "\n";

	std::string path = config.publicHtmlPath + request.path;

	uint8_t *fileData = nullptr;
	uint64_t fileSize = 0;
	
	if(stw::file::is_within_directory(path, config.publicHtmlPath))
	{
		if(fileCache.read_file(path, &fileData, &fileSize))
		{
			stw::memory_stream stream(fileData, fileSize);
			std::string contentType = stw::get_http_content_type(path);
			connection->write_response(200, nullptr, &stream, contentType);
			return;
		}
	}

	if(fileCache.read_file(config.privateHtmlPath + "/404.html", &fileData, &fileSize))
	{
		stw::memory_stream stream(fileData, fileSize);
		connection->write_response(404, nullptr, &stream, "text/html");
		return;
	}

	connection->write_response(404);
}