#include "web_app.hpp"
#include <iostream>

web_app::web_app()
{
	stw::signal::register_handler([this](int n) {
		if(n == SIGINT)
			server.stop();
	#ifndef _WIN32
		if(n == SIGPIPE)
			std::cerr << "Broken pipe\n";
	#endif
	});
	
	stw::signal::register_signal(SIGINT);
#ifndef _WIN32
	stw::signal::register_signal(SIGPIPE);
#endif
}

void web_app::run()
{
	if(!config.load_from_file("config.ini"))
		return;

	router.add(stw::http_method_get, "/", [&] (stw::http_connection *c, const stw::http_request_info &r) {
		std::string filePath = config.publicHtmlPath + "/index.html";
		send_file_content(c, filePath);
	});

	server.onRequest = [this] (stw::http_connection *c, const stw::http_request_info &r) {
		on_request(c, r);
	};

	server.run(config);
}

void web_app::on_request(stw::http_connection *connection, const stw::http_request_info &request)
{
	//std::cout << connection->get_ip() << " [" << stw::http_method_to_string(request.method) << "]: " << request.path << "\n";

	// Check if a suitable route can be found for this request
	auto route = router.get(request.path);

	if(route)
	{
		if(route->method == request.method)
			route->handler(connection, request);
		else
			connection->write_response(stw::http_status_code_method_not_allowed);
	}
	else
	{
		// If no route was found, the user may have requested a file instead
		std::string path = config.publicHtmlPath + request.path;

		// Checks if the file exists, and if it's inside the public html directory or any of its children
		if(stw::file::is_within_directory(path, config.publicHtmlPath))
			send_file_content(connection, path);
		else
			send_file_content(connection, config.privateHtmlPath + "/404.html");
	}
}

void web_app::send_file_content(stw::http_connection *connection, const std::string &filePath)
{
	constexpr size_t MAX_FILE_SIZE = 1024 * 1024;
	size_t fileSize = stw::file::get_size(filePath);

	static const stw::http_headers headers = {
		{ "Cache-Control", "max-age=3600" }
	};

	// Only files with size less than or equal to MAX_FILE_SIZE get cached
	if(fileSize <= MAX_FILE_SIZE)
	{
		uint8_t *fileData = nullptr;
		uint64_t fileSize = 0;

		if(fileCache.read_file(filePath, &fileData, &fileSize))
		{
			std::string contentType = stw::get_http_content_type(filePath);
			connection->write_response(stw::http_status_code_ok, &headers, fileData, fileSize, contentType);
			return;
		}
	}
	else
	{
		if(fileSize > 0)
		{
			stw::file_stream stream(filePath, stw::file_access_read);
			std::string contentType = stw::get_http_content_type(filePath);
			connection->write_response(stw::http_status_code_ok, &headers, &stream, contentType);
			return;
		}
	}

	connection->write_response(stw::http_status_code_not_found);
}