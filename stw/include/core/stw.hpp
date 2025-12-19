#ifndef STW_STW_HPP
#define STW_STW_HPP

namespace stw
{
	// Loads dynamic libraries such as curl/openssl/zlib. Call this method before using the library
	void load_library();
}

#endif