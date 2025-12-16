#include "stw_ini_reader.hpp"
#include "stw_string.hpp"
#include "stw_file.hpp"
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace stw
{
	bool ini_reader::field::try_get_uint16(uint16_t &v)
	{
		return string::try_parse_uint16(value, v);
	}

	bool ini_reader::field::try_get_int16(int16_t &v)
	{
		return string::try_parse_int16(value, v);
	}

	bool ini_reader::field::try_get_uint32(uint32_t &v)
	{
		return string::try_parse_uint32(value, v);
	}

	bool ini_reader::field::try_get_int32(int32_t &v)
	{
		return string::try_parse_int32(value, v);
	}

	bool ini_reader::field::try_get_uint64(uint64_t &v)
	{
		return string::try_parse_uint64(value, v);
	}

	bool ini_reader::field::try_get_int64(int64_t &v)
	{
		return string::try_parse_int64(value, v);
	}

	bool ini_reader::field::try_get_boolean(bool &b)
	{
        std::string lowercaseValue = string::to_lower(value);
        
        if (lowercaseValue == "true") 
		{
            b = true;
            return true;
        } 
		else if (lowercaseValue == "false") 
		{
            b = false;
            return true;
        } 
		else if(lowercaseValue == "1") 
		{
            b = true;
            return true;
		} 
		else if(lowercaseValue == "0") 
		{
            b = false;
            return true;
		}

        return false;
	}

	void ini_reader::add_required_field(const std::string &name, field_type type)
	{
		if (requiredFields.contains(name))
			throw std::runtime_error("A field with the same name already exists: " + name);
		requiredFields[name] = type;
	}

	std::unordered_map<std::string, ini_reader::field> ini_reader::read_file(const std::string &filePath)
	{
		if (requiredFields.size() == 0)
			throw std::runtime_error("No required fields have been set up");
		
		if (!file::exists(filePath))
			throw std::runtime_error("The given file path does not exist: " + filePath);

		std::vector<std::string> lines = file::read_all_lines(filePath);

		if (lines.size() == 0)
			throw std::runtime_error("The given file has no content");

		std::unordered_map<std::string, field> fields;

		for (size_t i = 0; i < lines.size(); i++)
		{
			lines[i] = strip_comment(lines[i]);

			if (lines[i].size() == 0)
				continue;

			auto parts = string::split(lines[i], ' ', 2); // Split on the first space only

			if (parts.size() < 2)
				continue;
			
			std::string &key = parts[0];
			std::string &value = parts[1];

			if (!requiredFields.contains(key))
				continue;

			if (fields.contains(key))
				throw std::runtime_error("The key already exists: " + key);

			field f = {
				.key = key,
				.value = value,
				.type = requiredFields[key]
			};

			fields[key] = f;
		}

		return fields;
	}

	std::string ini_reader::strip_comment(const std::string &line)
	{
		size_t commentIndex = line.find('#');

		// If # found, extract the substring before it
		std::string result = (commentIndex != std::string::npos) ? 
			line.substr(0, commentIndex) : line;

		// Trim leading and trailing spaces
		result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) {
			return !std::isspace(ch);
		}));
		
		result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
		}).base(), result.end());

		return result;
	}
}