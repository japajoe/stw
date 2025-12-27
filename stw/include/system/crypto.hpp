#ifndef STW_CRYPTO_HPP
#define STW_CRYPTO_HPP

#include <string>
#include <cstdint>
#include <cstdlib>

namespace stw::crypto
{
	std::string base64_encode(const uint8_t *buffer, size_t size);
	uint8_t *create_sha1_hash(const uint8_t *d, size_t n, uint8_t *md);
}

#endif