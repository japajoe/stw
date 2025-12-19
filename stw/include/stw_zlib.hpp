#ifndef STW_ZLIB_HPP
#define STW_ZLIB_HPP

#include <string>

namespace stw::zlib
{
	bool compress_data(const std::string &data, std::string &compressedData);
}

#endif