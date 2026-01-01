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

#ifndef STW_CRYPTO_HPP
#define STW_CRYPTO_HPP

#include <string>
#include <cstdint>
#include <cstdlib>

namespace stw::crypto
{
	std::string base64_encode(const uint8_t *buffer, size_t size);
	uint8_t *create_sha1_hash(const uint8_t *d, size_t n, uint8_t *md);
	uint8_t *create_sha_256(const uint8_t *password, size_t passwordLength, const uint8_t *salt, size_t saltLength, uint32_t iterations, uint32_t keyLength, uint8_t *output);
}

#endif