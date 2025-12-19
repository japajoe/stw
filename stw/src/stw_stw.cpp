#include "stw_stw.hpp"
#include "stw_http_client.hpp"
#include "stw_ssl.hpp"
#include "stw_zlib.hpp"
#include <iostream>

namespace stw
{
	void load_library()
	{
		if(!zlib::load_library())
			std::cout << "Failed to load zlib, compression functionality not available\n";

		if(!openssl::load_library())
			std::cout << "Failed to load openssl, SSL functionality not available\n";

		if(!curl::load_library())
			std::cout << "Failed to load curl, http client functionality not available\n";
	}
}