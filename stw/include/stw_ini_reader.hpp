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

#ifndef STW_INI_READER_HPP
#define STW_INI_READER_HPP

#include <string>
#include <unordered_map>
#include <cstdint>

namespace stw
{
	class ini_reader
	{
	public:
		enum field_type
		{
			field_type_number,
			field_type_string,
			field_type_boolean
		};
		
		struct field
		{
			std::string key;
			std::string value;
			field_type type;
			bool try_get_uint16(uint16_t &v);
			bool try_get_int16(int16_t &v);
			bool try_get_uint32(uint32_t &v);
			bool try_get_int32(int32_t &v);
			bool try_get_uint64(uint64_t &v);
			bool try_get_int64(int64_t &v);
			bool try_get_boolean(bool &b);
		};
		void add_required_field(const std::string &name, field_type type);
		std::unordered_map<std::string, field> read_file(const std::string &filePath);
	private:
		std::unordered_map<std::string, field_type> requiredFields;
		std::string strip_comment(const std::string &line);
	};
}

#endif