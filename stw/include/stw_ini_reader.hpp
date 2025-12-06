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