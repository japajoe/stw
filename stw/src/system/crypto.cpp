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
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <random>

namespace stw::crypto
{
	std::string create_uuid()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 255);

		std::ostringstream uuidStream;

		for (int i = 0; i < 16; ++i) {
			if (i == 6) 
				uuidStream << std::hex << ((dis(gen) % 16) | 0x40); // Set version to 0100 (Version 4)
			else if (i == 8)
				uuidStream << std::hex << ((dis(gen) % 4) | 0x80); // Set variant to 10xx
			else
				uuidStream << std::hex << dis(gen);

			if (i == 3 || i == 5 || i == 7 || i == 9)
				uuidStream << '-';
		}

		return uuidStream.str();
	}

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

	struct sha256_context
	{
		uint32_t state[8];
		uint32_t total[2];
		uint8_t buffer[64];
	};

	#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
	#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
	#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
	#define SIGMA0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
	#define SIGMA1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
	#define sigma0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
	#define sigma1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

	static const uint32_t K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	static void sha256_transform(sha256_context *ctx, const uint8_t data[64])
	{
		uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
		for (i = 0, j = 0; i < 16; ++i, j += 4)
			m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
		for (; i < 64; ++i)
			m[i] = sigma1(m[i - 2]) + m[i - 7] + sigma0(m[i - 15]) + m[i - 16];
		a = ctx->state[0];
		b = ctx->state[1];
		c = ctx->state[2];
		d = ctx->state[3];
		e = ctx->state[4];
		f = ctx->state[5];
		g = ctx->state[6];
		h = ctx->state[7];
		for (i = 0; i < 64; ++i)
		{
			t1 = h + SIGMA1(e) + CH(e, f, g) + K[i] + m[i];
			t2 = SIGMA0(a) + MAJ(a, b, c);
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}
		ctx->state[0] += a;
		ctx->state[1] += b;
		ctx->state[2] += c;
		ctx->state[3] += d;
		ctx->state[4] += e;
		ctx->state[5] += f;
		ctx->state[6] += g;
		ctx->state[7] += h;
	}

	static void sha256_init(sha256_context *ctx)
	{
		ctx->state[0] = 0x6a09e667;
		ctx->state[1] = 0xbb67ae85;
		ctx->state[2] = 0x3c6ef372;
		ctx->state[3] = 0xa54ff53a;
		ctx->state[4] = 0x510e527f;
		ctx->state[5] = 0x9b05688c;
		ctx->state[6] = 0x1f83d9ab;
		ctx->state[7] = 0x5be0cd19;
		ctx->total[0] = ctx->total[1] = 0;
	}

	static void sha256_update(sha256_context *ctx, const uint8_t *data, size_t len)
	{
		for (size_t i = 0; i < len; ++i)
		{
			ctx->buffer[ctx->total[0] & 0x3F] = data[i];
			if (++ctx->total[0] == 0)
				++ctx->total[1];
			if ((ctx->total[0] & 0x3F) == 0)
				sha256_transform(ctx, ctx->buffer);
		}
	}

	static void sha256_final(sha256_context *ctx, uint8_t hash[32])
	{
		uint32_t i = ctx->total[0] & 0x3F;
		ctx->buffer[i++] = 0x80;
		if (i > 56)
		{
			while (i < 64)
				ctx->buffer[i++] = 0x00;
			sha256_transform(ctx, ctx->buffer);
			i = 0;
		}
		while (i < 56)
			ctx->buffer[i++] = 0x00;
		uint64_t bits = ((uint64_t)ctx->total[1] << 32 | ctx->total[0]) << 3;
		for (int j = 0; j < 8; j++)
			ctx->buffer[56 + j] = (uint8_t)(bits >> (56 - j * 8));
		sha256_transform(ctx, ctx->buffer);
		for (i = 0; i < 8; ++i)
		{
			hash[i * 4] = (ctx->state[i] >> 24) & 0xff;
			hash[i * 4 + 1] = (ctx->state[i] >> 16) & 0xff;
			hash[i * 4 + 2] = (ctx->state[i] >> 8) & 0xff;
			hash[i * 4 + 3] = ctx->state[i] & 0xff;
		}
	}

	static void hmac_sha256(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len, uint8_t mac[32])
	{
		uint8_t k_ipad[64], k_opad[64], key_hash[32];
		sha256_context ctx;
		if (key_len > 64)
		{
			sha256_init(&ctx);
			sha256_update(&ctx, key, key_len);
			sha256_final(&ctx, key_hash);
			key = key_hash;
			key_len = 32;
		}
		std::memset(k_ipad, 0x36, 64);
		std::memset(k_opad, 0x5c, 64);
		for (size_t i = 0; i < key_len; i++)
		{
			k_ipad[i] ^= key[i];
			k_opad[i] ^= key[i];
		}
		sha256_init(&ctx);
		sha256_update(&ctx, k_ipad, 64);
		sha256_update(&ctx, data, data_len);
		sha256_final(&ctx, mac);
		sha256_init(&ctx);
		sha256_update(&ctx, k_opad, 64);
		sha256_update(&ctx, mac, 32);
		sha256_final(&ctx, mac);
	}

	uint8_t *create_sha_256(const uint8_t *password, size_t passwordLength, const uint8_t *salt, size_t saltLength, uint32_t iterations, uint32_t keyLength, uint8_t *output)
	{
		uint8_t U[32], T[32];
		uint32_t blocks = (keyLength + 31) / 32;
		for (uint32_t b = 1; b <= blocks; b++)
		{
			uint8_t salt_plus_i[saltLength + 4];
			std::memcpy(salt_plus_i, salt, saltLength);
			salt_plus_i[saltLength] = (b >> 24) & 0xff;
			salt_plus_i[saltLength + 1] = (b >> 16) & 0xff;
			salt_plus_i[saltLength + 2] = (b >> 8) & 0xff;
			salt_plus_i[saltLength + 3] = b & 0xff;

			hmac_sha256(password, passwordLength, salt_plus_i, saltLength + 4, U);
			std::memcpy(T, U, 32);
			for (uint32_t i = 1; i < iterations; i++)
			{
				hmac_sha256(password, passwordLength, U, 32, U);
				for (int j = 0; j < 32; j++)
					T[j] ^= U[j];
			}
			uint32_t chunk = (keyLength - (b - 1) * 32 < 32) ? (keyLength % 32) : 32;
			std::memcpy(output + (b - 1) * 32, T, chunk);
		}
		return output;
	}

	std::vector<uint8_t> create_salt(size_t length)
	{
		std::vector<uint8_t> salt(length);
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<uint8_t> dis(0, 255);

		for (size_t i = 0; i < length; ++i) 
		{
			salt[i] = dis(gen);
		}

		return salt;
	}

	bool password_set::create(const std::string &password, size_t saltLength, size_t hashLength, size_t iterations)
	{
		if(password.empty() || saltLength < 1 || hashLength < 1 || iterations < 1)
			return false;
		
		passwordSalt = create_salt(saltLength);
		passwordHash.resize(hashLength);
		
		stw::crypto::create_sha_256((uint8_t*)password.data(), password.size(), passwordSalt.data(), passwordSalt.size(), iterations, hashLength, passwordHash.data());
		
		return true;
	}

	bool password_set::match(const std::string &password)
	{
		if(passwordSalt.empty() || passwordHash.empty() || password.empty())
			return false;
		
		std::vector<uint8_t> hash;
		hash.resize(passwordSalt.size());		
		create_sha_256((uint8_t*)password.data(), password.size(), passwordSalt.data(), passwordSalt.size(), 2048, hash.size(), hash.data());
		return std::memcmp(hash.data(), passwordHash.data(), hash.size()) == 0;
	}
}