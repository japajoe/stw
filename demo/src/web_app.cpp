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

#include "web_app.hpp"
#include <iostream>

web_app::web_app()
{
	stw::signal::register_handler([this](int n) {
		if(n == SIGINT)
		{
			server.stop();
			exit(0);
		}
		if(n == SIGTERM)
		{
			server.stop();
		}
	#ifndef STW_PLATFORM_WINDOWS
		if(n == SIGPIPE) 
		{
			//std::cout << "Broken pipe\n";
		}
	#endif

	});
	
	stw::signal::register_signal(SIGINT);
	stw::signal::register_signal(SIGTERM);
#ifndef STW_PLATFORM_WINDOWS
	stw::signal::register_signal(SIGPIPE);
#endif
}

void web_app::run()
{
	if(!config.load_from_file("config.ini"))
		return;

	// This regex matches to "/" or "/index" and is case insensitive
	router.add(stw::http_method_get, std::regex("^/$|^/index$", std::regex_constants::icase), [&] (stw::http_connection *c, const stw::http_request_info &r) {
		std::string filePath = config.publicHtmlPath + "/index.html";
		send_file_content(c, stw::http_status_code_ok, filePath);
	});

	router.add(stw::http_method_post, "/api/v1/test", [&] (stw::http_connection *c, const stw::http_request_info &r) {
		if(r.contentLength > 0)
		{
			std::string requestText;
			requestText.resize(r.contentLength);
			if(c->read_all(requestText.data(), r.contentLength))
			{
				std::cout << requestText << '\n';
			}
		}
		c->write_response(200);
	});

	server.onRequest = [this] (stw::http_connection *c, const stw::http_request_info &r) {
		on_request(c, r);
	};

	server.run(config);
}

void web_app::on_request(stw::http_connection *connection, const stw::http_request_info &request)
{
	std::cout << connection->get_ip() << " [" << stw::http_method_to_string(request.method) << "]: " << request.path << "\n";

	if(!router.process_request(request.path, connection, request))
	{
		// If no route was found, the user may have requested a file instead
		std::string path = config.publicHtmlPath + request.path;

		// Checks if the file exists, and if it's inside the public html directory or any of its children
		if(stw::file::is_within_directory(path, config.publicHtmlPath))
			send_file_content(connection, stw::http_status_code_ok, path);
		else
			send_file_content(connection, stw::http_status_code_not_found, config.privateHtmlPath + "/404.html");
	}
}

void web_app::send_file_content(stw::http_connection *connection, uint32_t statusCode, const std::string &filePath)
{
	constexpr size_t MAX_FILE_SIZE = 1024 * 1024;
	size_t fileSize = stw::file::get_size(filePath);
	std::string contentType = stw::get_http_content_type(filePath);

	static const stw::http_headers headers = {
		{ "Cache-Control", "max-age=3600" }
	};

	// Only files with size less than or equal to MAX_FILE_SIZE get cached
	if(fileSize <= MAX_FILE_SIZE)
	{
		uint8_t *data = nullptr;
		uint64_t size = 0;

		if(fileCache.read_file(filePath, &data, &size))
		{
			connection->write_response(statusCode, &headers, data, size, contentType);
			return;
		}
	}
	else
	{
		if(fileSize > 0)
		{
			stw::file_stream stream(filePath, stw::file_access_read);
			connection->write_response(statusCode, &headers, &stream, contentType);
			return;
		}
	}

	connection->write_response(stw::http_status_code_not_found);
}