#include "stw.hpp"
#include "../net/http_client.hpp"
#include "../net/ssl.hpp"
#include <iostream>

namespace stw
{
	void load_library()
	{
		if(!openssl::load_library())
			std::cout << "Failed to load openssl, SSL functionality not available\n";

		if(!curl::load_library())
			std::cout << "Failed to load curl, http client functionality not available\n";
	}
}