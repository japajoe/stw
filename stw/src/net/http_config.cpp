#include "http_config.hpp"
#include "../system/ini_reader.hpp"
#include <iostream>

namespace stw
{
	void http_config::load_default() 
	{
		portHttp = 8080;
		portHttps = 8081;
		maxHeaderSize = 16384;
		bindAddress = "0.0.0.0";
		certificatePath = "cert.pem";
		privateKeyPath = "key.pem";
		publicHtmlPath = "www/public_html";
		privateHtmlPath = "www/private_html";
		hostName = "localhost";
		useHttpsForwarding = true;
	}

	bool http_config::load_from_file(const std::string &filePath)
	{
		ini_reader reader;
		reader.add_required_field("port_http", ini_reader::field_type_number);
		reader.add_required_field("port_https", ini_reader::field_type_number);
		reader.add_required_field("max_header_size", ini_reader::field_type_number);
		reader.add_required_field("bind_address", ini_reader::field_type_string);
		reader.add_required_field("certificate_path", ini_reader::field_type_string);
		reader.add_required_field("private_key_path", ini_reader::field_type_string);
		reader.add_required_field("public_html_path", ini_reader::field_type_string);
		reader.add_required_field("private_html_path", ini_reader::field_type_string);
		reader.add_required_field("host_name", ini_reader::field_type_string);
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

			if(!fields["port_http"].try_get_uint16(portHttp))
				return false;
			if(!fields["port_https"].try_get_uint16(portHttps))
				return false;
			if(!fields["max_header_size"].try_get_uint32(maxHeaderSize))
				return false;
			if(!fields["use_https_forwarding"].try_get_boolean(useHttpsForwarding))
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