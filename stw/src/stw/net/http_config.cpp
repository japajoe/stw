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

#include "http_config.hpp"
#include "../system/ini_reader.hpp"
#include <iostream>

namespace stw
{
	void http_config::load_default() 
	{
		port = 8080;
		maxHeaderSize = 16384;
		bindAddress = "0.0.0.0";
		publicHtmlPath = "www/public_html";
		privateHtmlPath = "www/private_html";
		hostName = "localhost";
	}

	bool http_config::load_from_file(const std::string &filePath)
	{
		ini_reader reader;
		reader.add_required_field("port", ini_reader::field_type_number);
		reader.add_required_field("max_header_size", ini_reader::field_type_number);
		reader.add_required_field("bind_address", ini_reader::field_type_string);
		reader.add_required_field("public_html_path", ini_reader::field_type_string);
		reader.add_required_field("private_html_path", ini_reader::field_type_string);
		reader.add_required_field("host_name", ini_reader::field_type_string);

		try
		{
			auto fields = reader.read_file(filePath);

			bindAddress = fields["bind_address"].value;
			publicHtmlPath = fields["public_html_path"].value;
			privateHtmlPath = fields["private_html_path"].value;
			hostName = fields["host_name"].value;

			if(!fields["port"].try_get_uint16(port))
				return false;
			if(!fields["max_header_size"].try_get_uint32(maxHeaderSize))
				return false;
			
			return true;
		}
		catch(const std::exception &e)
		{
			std::cout << e.what() << '\n';
			return false;
		}
	}
}