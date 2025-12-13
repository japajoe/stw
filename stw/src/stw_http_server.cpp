#include "stw_http_server.hpp"
#include "stw_ini_reader.hpp"
#include "stw_directory.hpp"
#include "stw_string.hpp"
#include <algorithm>
#include <iostream>
#include <future>
#include <utility>
#include <cstring>
#include <sstream>
#include <thread>

namespace stw
{
	http_server::http_server()
	{
		quit.store(false);
	}

	http_server::~http_server()
	{
		for(auto &listener : listeners)
			listener.close();
	}

	void http_server::run(http_server_configuration &config)
	{
		if(quit.load() == true)
			return;

		this->config = config;

		if(!directory::exists(config.publicHtmlPath))
			directory::create(config.publicHtmlPath);
		
		if(!directory::exists(config.privateHtmlPath))
			directory::create(config.privateHtmlPath);

		if(listeners.size() > 0)
		{
			for(auto &listener : listeners)
				listener.close();
			listeners.clear();
		}

		listeners.emplace_back();
		
		if(config.useHttps)
		{
			if(!sslContext.create(config.certificatePath, config.privateKeyPath))
				return;
			listeners.emplace_back();
		}

		auto httpTask = std::async(std::launch::async, &http_server::accept_http, this);
		(void)httpTask;

		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if(config.useHttps)
		{
			auto httpsTask = std::async(std::launch::async, &http_server::accept_https, this);
			(void)httpsTask;
		}
	}

	void http_server::stop()
	{
		quit.store(true);
	}

	void http_server::accept_http()
	{
		auto &listener = listeners[0];

		if(!listener.bind(config.bindAddress, config.port))
			return;

		if(!listener.listen(100))
			return;
			
		listener.set_timeout(1);
		
		std::cout << "Server listening on: http://" << config.bindAddress << ':' << config.port << '\n';

		while(!quit.load())
		{
			socket client;
			
			if(listener.accept(&client))
			{
				http_connection *connection = new http_connection(client);

				if(connection != nullptr)
				{
					threadPool.enqueue([this, c = connection] () {
						on_request(c);
					});
				}
				else
				{
					http_connection con(client);
					con.write_response(http_status_code_service_unavailable);
					con.close();
				}
			}
		}

		listener.close();
	}

	void http_server::accept_https()
	{
		auto &listener = listeners[1];

		if(!listener.bind(config.bindAddress, config.portHttps))
			return;

		if(!listener.listen(100))
			return;
			
		listener.set_timeout(1);
		
		std::cout << "Server listening on: https://" << config.bindAddress << ':' << config.portHttps << '\n';

		while(!quit.load())
		{
			socket client;
			
			if(listener.accept(&client))
			{
				ssl s;

				if(!s.create(&sslContext))
				{
					client.close();
					continue;
				}

				if(!s.set_file_descriptor(client.get_file_descriptor()))
				{
					client.close();
					continue;
				}

				if(!s.accept())
				{
					client.close();
					continue;
				}

				http_connection *connection = new http_connection(client, s);

				if(connection != nullptr)
				{
					threadPool.enqueue([this, c = connection] () {
						on_request(c);
					});
				}
				else
				{
					http_connection con(client, s);
					con.write_response(http_status_code_service_unavailable);
					con.close();
				}
			}
		}

		listener.close();

		sslContext.destroy();
	}

	void http_server::on_request(http_connection *connection)
	{
		std::string headerText;

		if(read_header(connection, headerText) != http_header_error_none)
		{
			connection->write_response(http_status_code_bad_request);
			connection->close();
			return;
		}

		http_headers headers;
		std::string method;
		std::string path;
		uint64_t contentLength;

		if(!parse_request_header2(headerText, headers, method, path, contentLength))
		{
			connection->write_response(http_status_code_bad_request);
			connection->close();
			return;
		}

		if(!connection->is_secure() && config.useHttps && config.useHttpsForwarding) 
		{
			if(headers.contains("upgrade-insecure-requests"))
			{
				const auto &upgrade = headers["upgrade-insecure-requests"];
				if(upgrade == "1")
				{
					http_headers responseHeaders;
					responseHeaders["Location"] = "https://" + config.hostName + ":" + std::to_string(config.portHttps) + path;
					responseHeaders["Connection"] = "close";
					connection->write_response(http_status_code_moved_permanently, &responseHeaders);
					connection->close();
					return;
				}
			}
		}

		if(onRequest)
		{
			http_method httpMethod = string_to_http_method(method);

			http_request_info request = {
				.headers = headers,
				.method = httpMethod,
				.path = path,
				.contentLength = contentLength
			};

			onRequest(connection, request);
		}
		else
		{
			connection->write_response(http_status_code_not_found);
		}
		
		connection->close();

		delete connection;
	}

	http_header_error http_server::read_header(http_connection *connection, std::string &header)
    {
        if(!connection)
            return http_header_error_failed_to_read;

        const size_t maxHeaderSize = config.maxHeaderSize;
		const size_t bufferSize = 4096;
		char buffer[bufferSize];
        int64_t headerEnd = 0;
        int64_t totalHeaderSize = 0;
        bool endFound = false;

        auto findHeaderEnd = [] (const char* haystack, const char* needle, size_t haystackLength, size_t needleLength) -> int64_t {
            for (size_t i = 0; i <= haystackLength - needleLength; ++i) 
			{
                if (std::memcmp(haystack + i, needle, needleLength) == 0)
                    return static_cast<int>(i); // Found the needle, return the index
            }
            return -1; // Not found
        };

        //char *pBuffer = buffer.data();
		char *pBuffer = buffer;

        // Peek to find the end of the header
        while (true) 
        {
            int64_t bytesPeeked = connection->peek(pBuffer, bufferSize);

            if (bytesPeeked < 0)
                return http_header_error_failed_to_peek;

            //Don't loop indefinitely...
            if(bytesPeeked == 0)
                break;

            totalHeaderSize += bytesPeeked;

            if(totalHeaderSize > maxHeaderSize) 
            {
                printf("header_error_max_size_exceeded [1], %zu/%zu\n", totalHeaderSize, maxHeaderSize);
                return http_header_error_max_size_exceeded;
            }
            
            // Look for the end of the header (double CRLF)
            int64_t end = findHeaderEnd(pBuffer, "\r\n\r\n", bytesPeeked, 4);

            if(end >= 0)
            {
                headerEnd = end + 4; //Include the length of the CRLF
                endFound = true;
                break;                
            }
        }

        if(!endFound)
            return http_header_error_end_not_found;

        // Now read the header
        header.resize(headerEnd);
        int64_t bytesRead = connection->read_all(header.data(), headerEnd);
        if (bytesRead <= 0) 
            return http_header_error_failed_to_read;

        if(header.size() > maxHeaderSize) 
        {
            printf("header_error_max_size_exceeded [2], %zu/%zu\n", header.size(), maxHeaderSize);
            return http_header_error_max_size_exceeded;
        }

        return http_header_error_none;
    }

	bool http_server::parse_request_header(const std::string &responseText, http_headers &header, std::string &method, std::string &path, uint64_t &contentLength)
    {
		contentLength = 0;

        std::istringstream responseStream(responseText);
        std::string line;
        size_t currentLine = 0;

        while(std::getline(responseStream, line))
        {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

            if(line.size() == 0)
                continue;

            if(currentLine == 0)
            {
                if(line[line.size() - 1] == ' ')
                    line.pop_back();

                auto parts = string::split(line, ' ', 0);
                
                if(parts.size() < 2)
                    return false;

                method = string::to_lower(parts[0]);
                path = parts[1];

				if(path.size() == 0)
					path = "/";
            }
            else
            {
                auto parts = string::split(line, ':', 2);

                if(parts.size() == 2)
                {
                    parts[0] = string::to_lower(parts[0]);
                    parts[1] = string::trim_start(parts[1]);
                    header[parts[0]] = parts[1];
                }
            }

            currentLine++;
        }

        if(header.contains("content-length"))
        {
            if(!string::try_parse_uint64(header["content-length"], contentLength))
                contentLength = 0;
        }

        return currentLine > 0;
    }

	bool http_server::parse_request_header2(const std::string &responseText, http_headers &header, std::string &method, std::string &path, uint64_t &contentLength)
	{
		size_t pos = 0;
		size_t end;

		// Find the end of the headers section (double CRLF)
		end = responseText.find("\r\n\r\n", pos);
		if (end == std::string::npos)
			return false; // No headers found

		// Parse the request line (first line before the headers)
		size_t requestLineEnd = responseText.find("\r\n", pos);
		
		if (requestLineEnd != std::string::npos && requestLineEnd < end) 
		{
			std::string requestLine = responseText.substr(pos, requestLineEnd - pos);
			std::istringstream requestStream(requestLine);
			
			// Extract method and path
			requestStream >> method >> path;

			// Check if method and path were extracted successfully
			if (method.empty() || path.empty())
				return false;

			pos = requestLineEnd + 2; // Move past the request line
		} 
		else
			return false; // Invalid request line

		// Initialize content length to 0
		contentLength = 0;

		// Iterate through the headers section
		size_t lineEnd;
		
		while ((lineEnd = responseText.find("\r\n", pos)) != std::string::npos && pos < end) 
		{
			// Get the header line
			std::string line = responseText.substr(pos, lineEnd - pos);
			pos = lineEnd + 2; // Move to the next line

			// Split the line into key and value
			size_t delimiterPos = line.find(": ");
			if (delimiterPos != std::string::npos) 
			{
				std::string key = line.substr(0, delimiterPos);
				std::string value = line.substr(delimiterPos + 2); // Skip the ": "

				// Insert into the headers map
				header[key] = value;

				// Check for Content-Length
				if (key == "Content-Length" || key == "content-length")
				{
					try 
					{
						contentLength = std::stoull(value);
					} 
					catch (const std::invalid_argument&) 
					{
						contentLength = 0; // Handle invalid value
					} 
					catch (const std::out_of_range&) 
					{
						contentLength = UINT64_MAX; // Handle out of range
					}
				}
			}
		}

		return true;
	}

	http_connection::http_connection(socket &connection)
	{
		this->connection = std::move(connection);
	}

	http_connection::http_connection(socket &connection, ssl &s)
	{
		this->connection = std::move(connection);
		this->s = std::move(s);
	}

	http_connection::http_connection(http_connection &&other) noexcept
	{
		connection = std::move(other.connection);
		s = std::move(other.s);
	}

	http_connection &http_connection::operator=(http_connection &&other) noexcept
	{
		if(this != &other)
		{
			connection = std::move(other.connection);
			s = std::move(other.s);
		}
		return *this;
	}

	int64_t http_connection::read(void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.read(buffer, size);
		return connection.read(buffer, size);
	}

	int64_t http_connection::write(const void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.write(buffer, size);
		return connection.write(buffer, size);
	}

	int64_t http_connection::peek(void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.peek(buffer, size);
		return connection.peek(buffer, size);
	}

	int64_t http_connection::read_all(void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.read_all(buffer, size);
		return connection.read_all(buffer, size);
	}

	int64_t http_connection::write_all(const void *buffer, size_t size)
	{
		if(s.is_valid())
			return s.write_all(buffer, size);
		return connection.write_all(buffer, size);
	}

	bool http_connection::write_response(uint32_t statusCode)
	{
		return write_response(statusCode, nullptr, nullptr, 0, "");
	}

	bool http_connection::write_response(uint32_t statusCode, const http_headers *headers)
	{
		return write_response(statusCode, headers, nullptr, 0, "");
	}

	bool http_connection::write_response(uint32_t statusCode, const http_headers *headers, stream *content, const std::string &contentType)
	{
		std::ostringstream responseText;
		responseText << "HTTP/1.1 " << statusCode << "\r\n";
		
		if(headers)
		{
			if(headers->size() > 0)
			{
				for(const auto &[key,value] : *headers)
				{
					responseText << key << ": " << value << "\r\n";
				}
			}
		}

		if(content != nullptr && contentType.size() > 0)
		{
			responseText << "Content-Length: " << content->get_length() << "\r\n";
			responseText << "Content-Type: " << contentType << "\r\n";
		}

		responseText << "Connection: close\r\n\r\n";

		if(write_all(responseText.str().data(), responseText.str().size()))
		{
			if(content != nullptr && content->get_length() > 0)
			{
				std::vector<uint8_t> buffer(8192);

				int64_t nBytes = 0;
				
				while((nBytes = content->read(buffer.data(), buffer.size())) > 0)
				{
					write(buffer.data(), nBytes);
				}
			}
			return true;
		}
		return false;
	}

	bool http_connection::write_response(uint32_t statusCode, const http_headers *headers, const void *content, uint64_t contentLength, const std::string &contentType)
	{
		std::ostringstream responseText;
		responseText << "HTTP/1.1 " << statusCode << "\r\n";

		if(headers)
		{
			if(headers->size() > 0)
			{
				for(const auto &[key,value] : *headers)
				{
					responseText << key << ": " << value << "\r\n";
				}
			}
		}

		if(content != nullptr && contentLength > 0 && contentType.size() > 0)
		{
			responseText << "Content-Length: " << contentLength << "\r\n";
			responseText << "Content-Type: " << contentType << "\r\n";
		}

		responseText << "Connection: close\r\n\r\n";

		if(write_all(responseText.str().data(), responseText.str().size()))
		{
			if(content != nullptr && contentLength > 0)
				return write_all(content, contentLength);
			return true;
		}
		return false;
	}

	void http_connection::close()
	{
		connection.close();
		s.destroy();
	}

	bool http_connection::is_secure() const
	{
		return s.is_valid();
	}

	std::string http_connection::get_ip() const
	{
		return connection.get_ip();
	}

	void http_server_configuration::load_default() 
	{
		port = 8080;
		portHttps = 8081;
		maxHeaderSize = 16384;
		bindAddress = "0.0.0.0";
		certificatePath = "cert.pem";
		privateKeyPath = "key.pem";
		publicHtmlPath = "www/public_html";
		privateHtmlPath = "www/private_html";
		hostName = "localhost";
		useHttps = true;
		useHttpsForwarding = true;
	}

	bool http_server_configuration::load_from_file(const std::string &filePath)
	{
		ini_reader reader;
		reader.add_required_field("port", ini_reader::field_type_number);
		reader.add_required_field("port_https", ini_reader::field_type_string);
		reader.add_required_field("max_header_size", ini_reader::field_type_number);
		reader.add_required_field("bind_address", ini_reader::field_type_string);
		reader.add_required_field("certificate_path", ini_reader::field_type_string);
		reader.add_required_field("private_key_path", ini_reader::field_type_string);
		reader.add_required_field("public_html_path", ini_reader::field_type_string);
		reader.add_required_field("private_html_path", ini_reader::field_type_string);
		reader.add_required_field("host_name", ini_reader::field_type_string);
		reader.add_required_field("use_https", ini_reader::field_type_boolean);
		reader.add_required_field("use_https_forwarding", ini_reader::field_type_boolean);

		try
		{
			auto fields = reader.read_file(filePath);

			bindAddress = fields["bind_address"].value;
			certificatePath = fields["certificate_path"].value;
			privateKeyPath = fields["private_key_path"].value;
			publicHtmlPath = fields["public_html_path"].value;
			privateHtmlPath = fields["private_html_path"].value;
			hostName = fields["host_name"].value;

			if(!fields["port"].try_get_uint16(port))
				return false;
			if(!fields["port_https"].try_get_uint16(portHttps))
				return false;
			if(!fields["max_header_size"].try_get_uint32(maxHeaderSize))
				return false;
			if(!fields["use_https"].try_get_boolean(useHttps))
				return false;
			if(!fields["use_https_forwarding"].try_get_boolean(useHttpsForwarding))
				return false;
			
			return true;
		}
		catch(const std::exception &e)
		{
			std::cerr << e.what() << '\n';
			return false;
		}
	}
}