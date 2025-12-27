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

#include "crypto.hpp"
#include <vector>
#include <cstring>

namespace stw::crypto
{
	static inline uint32_t left_rotate(uint32_t value, size_t bits)
	{
		return (value << bits) | (value >> (32 - bits));
	}

	std::string base64_encode(const uint8_t *buffer, size_t size)
	{
		static const char lookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		std::string out;
		int32_t val = 0;
		int32_t valb = -6;
		for (size_t i = 0; i < size; i++)
		{
			val = (val << 8) + buffer[i];
			valb += 8;
			while (valb >= 0)
			{
				out.push_back(lookup[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}
		if (valb > -6)
			out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]);
		while (out.size() % 4)
			out.push_back('=');
		return out;
	}

	uint8_t *create_sha1_hash(const uint8_t *d, size_t n, uint8_t *md)
	{
		uint32_t h0 = 0x67452301;
		uint32_t h1 = 0xEFCDAB89;
		uint32_t h2 = 0x98BADCFE;
		uint32_t h3 = 0x10325476;
		uint32_t h4 = 0xC3D2E1F0;

		// Pre-processing: Padding the message
		std::vector<uint8_t> msg(d, d + n);
		uint64_t bitLength = n * 8;
		msg.push_back(0x80);
		while ((msg.size() + 8) % 64 != 0)
			msg.push_back(0x00);
		for (int i = 7; i >= 0; i--)
			msg.push_back((bitLength >> (i * 8)) & 0xFF);

		// Process in 512-bit chunks
		for (size_t i = 0; i < msg.size(); i += 64)
		{
			uint32_t w[80];
			for (uint32_t j = 0; j < 16; j++)
			{
				w[j] = (msg[i + j * 4] << 24) | (msg[i + j * 4 + 1] << 16) |
					   (msg[i + j * 4 + 2] << 8) | (msg[i + j * 4 + 3]);
			}
			for (uint32_t j = 16; j < 80; j++)
				w[j] = left_rotate(w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16], 1);

			uint32_t a = h0, b = h1, c = h2, d_v = h3, e = h4;

			for (uint32_t j = 0; j < 80; j++)
			{
				uint32_t f, k;
				if (j < 20)
				{
					f = (b & c) | ((~b) & d_v);
					k = 0x5A827999;
				}
				else if (j < 40)
				{
					f = b ^ c ^ d_v;
					k = 0x6ED9EBA1;
				}
				else if (j < 60)
				{
					f = (b & c) | (b & d_v) | (c & d_v);
					k = 0x8F1BBCDC;
				}
				else
				{
					f = b ^ c ^ d_v;
					k = 0xCA62C1D6;
				}

				uint32_t temp = left_rotate(a, 5) + f + e + k + w[j];
				e = d_v;
				d_v = c;
				c = left_rotate(b, 30);
				b = a;
				a = temp;
			}
			h0 += a;
			h1 += b;
			h2 += c;
			h3 += d_v;
			h4 += e;
		}

		uint32_t states[] = {h0, h1, h2, h3, h4};
		for (int i = 0; i < 5; i++)
		{
			md[i * 4] = (states[i] >> 24) & 0xFF;
			md[i * 4 + 1] = (states[i] >> 16) & 0xFF;
			md[i * 4 + 2] = (states[i] >> 8) & 0xFF;
			md[i * 4 + 3] = states[i] & 0xFF;
		}
		return md;
	}
}