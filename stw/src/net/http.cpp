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

#include "http.hpp"
#include "../system/string.hpp"
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace stw
{
	http_request::http_request()
	{
		contentLength = 0;
		method = http_method_unknown;
	}

	bool http_request::parse(const std::string &requestBody, http_request &request)
	{
		if(request.headers.size() > 0)
			request.headers.clear();
		request.contentLength = 0;

		size_t pos = 0;
		size_t end;

		// Find the end of the headers
		end = requestBody.find("\r\n\r\n", pos);
		if (end == std::string::npos)
			return false; // No end of header found

		// Parse the request line (first line before the headers)
		size_t requestLineEnd = requestBody.find("\r\n", pos);
		
		if (requestLineEnd != std::string::npos && requestLineEnd < end) 
		{
			std::string requestLine = requestBody.substr(pos, requestLineEnd - pos);
			std::istringstream requestStream(requestLine);
			
			// Extract method/path/http version
			std::string method;
			requestStream >> method >> request.path >> request.httpVersion;

			if (method.empty() || request.path.empty() || request.httpVersion.empty())
				return false;
			
			request.method = get_http_method_from_string(method);

			pos = requestLineEnd + 2; // Move past the request line
		} 
		else
		{
			return false; // Invalid request line
		}

		try
		{
			request.path = string::url_decode(request.path);
		}
		catch(...)
		{
			return false;
		}

		// We don't want anything other than UTF8 in the path
		if(!string::is_valid_utf8(request.path.data(), request.path.size()))
			return false;

		size_t lineEnd;
		bool contentLengthSet = false;
		
		while ((lineEnd = requestBody.find("\r\n", pos)) != std::string::npos && pos < end) 
		{
			// Get the header line
			std::string line = requestBody.substr(pos, lineEnd - pos);
			pos = lineEnd + 2; // Move to the next line

			// Split the line into key and value
			size_t delimiterPos = line.find(": ");
			if (delimiterPos != std::string::npos) 
			{
				// Convert key to lower case because some clients may be messy
				std::string key = line.substr(0, delimiterPos);

				for(auto &c : key) 
					c = std::tolower(c);

				std::string value = line.substr(delimiterPos + 2); // Skip the ": "

				if(key == "content-length")
				{
					try 
					{
						request.contentLength = std::stoull(value);
						contentLengthSet = true;
					} 
					catch (...) 
					{
						request.contentLength = 0;
						contentLengthSet = true;
					}
				}

				// Insert into the headers map
				request.headers[std::move(key)] = std::move(value);
			}
		}

		// Just in case that there might not be a content-length header
		if(!contentLengthSet)
			request.contentLength = 0;

		return true;
	}


	static inline bool compare_case_insensitive(const std::string &str1, const std::string &str2) 
	{
		if (str1.length() != str2.length())
			return false;

		for (size_t i = 0; i < str1.length(); ++i) 
		{
			if (std::tolower(static_cast<unsigned char>(str1[i])) != std::tolower(static_cast<unsigned char>(str2[i])))
				return false;
		}
		return true; // All characters matched
	}

	http_method http_request::get_http_method_from_string(const std::string &method)
	{
        if(compare_case_insensitive(method, "GET"))
            return http_method_get;
        else if(compare_case_insensitive(method, "POST"))
            return http_method_post;
        else if(compare_case_insensitive(method, "PUT"))
            return http_method_put;
        else if(compare_case_insensitive(method, "PATCH"))
            return http_method_patch;
        else if(compare_case_insensitive(method, "DELETE"))
            return http_method_delete;
        else if(compare_case_insensitive(method, "HEAD"))
            return http_method_head;
        else if(compare_case_insensitive(method, "OPTIONS"))
            return http_method_options;
        else if(compare_case_insensitive(method, "TRACE"))
            return http_method_trace;
        else if(compare_case_insensitive(method, "CONNECT"))
            return http_method_connect;
        else
            return http_method_unknown;
	}

	std::string http_request::get_string_from_http_method(http_method method)
	{
        switch(method)
        {
        case http_method_get:
            return "GET";
        case http_method_post:
            return "POST";
        case http_method_put:
            return "PUT";
        case http_method_patch:
            return "PATCH";
        case http_method_delete:
            return "DELETE";
        case http_method_head:
            return "HEAD";
        case http_method_options:
            return "OPTIONS";
        case http_method_trace:
            return "TRACE";
        case http_method_connect:
            return "CONNECT";
        default:
            return "UNKNOWN";
        }
	}

	http_response::http_response()
	{
		statusCode = 0;
		content = nullptr;
	}

    static std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".bmp", "image/bmp"},
        {".svg", "image/svg+xml"},
        {".pdf", "application/pdf"},
        {".txt", "text/plain"},
        {".xml", "application/xml"}
    };

    std::string get_http_content_type(const std::string &filePath)
    {
        std::filesystem::path path(filePath);
        std::string extension = path.extension().string();

        auto it = mimeTypes.find(extension);
        if (it != mimeTypes.end())
            return it->second;
        
        return "application/octet-stream";
    }
}