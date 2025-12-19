#ifndef STW_ZLIB_HPP
#define STW_ZLIB_HPP

#include <string>

namespace stw::zlib
{
	enum compression_algorithm
	{
		compression_algorithm_deflate,
		compression_algorithm_gzip
	};

	bool load_library();
	bool compress_data(const std::string &data, std::string &compressedData, compression_algorithm algorithm);
}

#endif