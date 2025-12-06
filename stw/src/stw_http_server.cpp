#include "stw_http_server.hpp"
#include "stw_ini_reader.hpp"
#include "stw_directory.hpp"
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

		auto task1 = std::async(std::launch::async, &http_server::accept_http, this);

		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if(config.useHttps)
			auto task2 = std::async(std::launch::async, &http_server::accept_https, this);
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

				if(threadPool.is_available())
				{
					threadPool.enqueue([this, c = connection] () {
						on_request(c);
					});
				}
				else
				{
					auto task = std::async(std::launch::async, &http_server::on_request, this, connection);
					(void)task;
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

				if(threadPool.is_available())
				{
					threadPool.enqueue([this, c = connection] () {
						on_request(c);
					});
				}
				else
				{
					auto task = std::async(std::launch::async, &http_server::on_request, this, connection);
					(void)task;
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

		if(!parse_request_header(headerText, headers, method, path, contentLength))
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
			connection->close();
		}
		else
		{
			connection->write_response(http_status_code_not_found);
			connection->close();
		}

		delete connection;
	}

	http_header_error http_server::read_header(http_connection *connection, std::string &header)
    {
        if(!connection)
            return http_header_error_failed_to_read;

        const size_t maxHeaderSize = config.maxHeaderSize;
        const size_t bufferSize = maxHeaderSize;
        std::vector<char> buffer;
        buffer.resize(bufferSize);
        int64_t headerEnd = 0;
        int64_t totalHeaderSize = 0;
        bool endFound = false;

        auto find_header_end = [] (const char* haystack, const char* needle, size_t haystackLength, size_t needleLength) -> int64_t {
            for (size_t i = 0; i <= haystackLength - needleLength; ++i) 
			{
                if (std::memcmp(haystack + i, needle, needleLength) == 0)
                    return static_cast<int>(i); // Found the needle, return the index
            }
            return -1; // Not found
        };

        char *pBuffer = buffer.data();

        // Peek to find the end of the header
        while (true) 
        {
            int64_t bytesPeeked = connection->peek(pBuffer, bufferSize);

            totalHeaderSize += bytesPeeked;

            if(totalHeaderSize > maxHeaderSize) 
            {
                printf("header_error_max_size_exceeded [1], %zu/%zu\n", totalHeaderSize, maxHeaderSize);
                return http_header_error_max_size_exceeded;
            }

            if (bytesPeeked < 0)
                return http_header_error_failed_to_peek;

            //Don't loop indefinitely...
            if(bytesPeeked == 0)
                break;
            
            // Look for the end of the header (double CRLF)
            int64_t end = find_header_end(pBuffer, "\r\n\r\n", bytesPeeked, 4);

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
        int64_t bytesRead = connection->read(header.data(), headerEnd);
        if (bytesRead < 0) 
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
        auto to_lower = [] (const std::string &str) -> std::string {
            std::string lower_str = str;
            std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                        [](unsigned char c) { return std::tolower(c); });
            return lower_str;
        };

		auto string_split = [] (const std::string& str, char separator, size_t maxParts) -> std::vector<std::string> {
			std::vector<std::string> result;
			size_t start = 0;
			size_t end = 0;

			while ((end = str.find(separator, start)) != std::string::npos) 
			{
				result.push_back(str.substr(start, end - start));
				start = end + 1;

				if (maxParts > 0 && result.size() >= maxParts - 1) 
					break;
			}
			result.push_back(str.substr(start));
			return result;
		};

		auto string_trim_start = [] (const std::string& str) -> std::string {
			size_t start = 0;
			while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) 
			{
				++start;
			}
			return str.substr(start);
		};

		auto try_parse_uint64 = [] (const std::string &value, uint64_t &v) -> bool {
			std::stringstream ss(value);
			ss >> v;

			if (ss.fail() || !ss.eof())
				return false;
			
			return true;
		};

        std::istringstream responseStream(responseText);
        std::string line;
        size_t count = 0;

        while(std::getline(responseStream, line))
        {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

            if(line.size() == 0)
                continue;

            if(count == 0)
            {
                if(line[line.size() - 1] == ' ')
                    line.pop_back();

                auto parts = string_split(line, ' ', 0);
                
                if(parts.size() < 2)
                    return false;

                method = to_lower(parts[0]);
                path = parts[1];

				if(path.size() == 0)
					path = "/";
            }
            else
            {
                auto parts = string_split(line, ':', 2);

                if(parts.size() == 2)
                {
                    parts[0] = to_lower(parts[0]);
                    parts[1] = string_trim_start(parts[1]);
                    header[parts[0]] = parts[1];
                }
            }

            count++;
        }

        if(header.contains("content-length"))
        {
            if(!try_parse_uint64(header["content-length"], contentLength))
                contentLength = 0;
        }

        return count > 0;
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

	http_connection::http_connection(const http_connection &other)
	{
		connection = other.connection;
		s = other.s;
	}

	http_connection::http_connection(http_connection &&other) noexcept
	{
		connection = std::move(other.connection);
		s = std::move(other.s);
	}

	http_connection &http_connection::operator=(const http_connection &other)
	{
		if(this != &other)
		{
			connection = other.connection;
			s = other.s;
		}
		return *this;
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
		std::string responseText = "HTTP/1.1 " + std::to_string(statusCode) + "\r\n";
		
		if(headers)
		{
			if(headers->size() > 0)
			{
				for(const auto &[key,value] : *headers)
				{
					responseText += key + ": " + value + "\r\n";
				}
			}
		}

		if(content != nullptr && contentType.size() > 0)
		{
			responseText += "Content-Length: " + std::to_string(content->get_length()) + "\r\n";
			responseText += "Content-Type: " + contentType + "\r\n";
		}

		responseText += "\r\n";

		if(write_all(responseText.data(), responseText.size()))
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
		std::string responseText = "HTTP/1.1 " + std::to_string(statusCode) + "\r\n";
		
		if(headers)
		{
			if(headers->size() > 0)
			{
				for(const auto &[key,value] : *headers)
				{
					responseText += key + ": " + value + "\r\n";
				}
			}
		}

		if(content != nullptr && contentLength > 0 && contentType.size() > 0)
		{
			responseText += "Content-Length: " + std::to_string(contentLength) + "\r\n";
			responseText += "Content-Type: " + contentType + "\r\n";
		}

		responseText += "\r\n";

		if(write_all(responseText.data(), responseText.size()))
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