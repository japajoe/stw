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

#ifndef STW_STRING_HPP
#define STW_STRING_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace stw::string
{
	bool is_valid_utf8(const void *data, size_t size);
	std::string url_decode(const std::string &str);
	bool compare(const std::string &str1, const std::string &str2, bool ignoreCase);
	bool contains(const std::string &haystack, const std::string &needle);
	bool starts_with(const std::string &haystack, const std::string &needle);
	bool ends_with(const std::string &haystack, const std::string &needle);
	std::string trim(const std::string& str);
	std::string trim_start(const std::string& str);
	std::string trim_end(const std::string& str);
	std::vector<std::string> split(const std::string& str, char separator, size_t maxParts = 0);
	std::string replace(const std::string &str, const std::string &target, const std::string &replacement);
	std::string to_lower(const std::string &str);
	std::string to_upper(const std::string &str);
	std::string sub_string(const std::string &str, size_t startIndex);
	std::string sub_string(const std::string &str, size_t startIndex, size_t length);
	int64_t index_of(const std::string &str, const std::string &subStr);
	bool try_parse_uint8(const std::string &value, uint8_t &v);
	bool try_parse_uint16(const std::string &value, uint16_t &v);
	bool try_parse_uint32(const std::string &value, uint32_t &v);
	bool try_parse_uint64(const std::string &value, uint64_t &v);
	bool try_parse_int8(const std::string &value, int8_t &v);
	bool try_parse_int16(const std::string &value, int16_t &v);
	bool try_parse_int32(const std::string &value, int32_t &v);
	bool try_parse_int64(const std::string &value, int64_t &v);
	bool try_parse_float(const std::string &value, float &v);
	bool try_parse_double(const std::string &value, double &v);
}

#endif