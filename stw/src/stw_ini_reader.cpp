#include "stw_ini_reader.hpp"
#include <stdexcept>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace stw
{
	static std::vector<std::string> read_all_lines(const std::string &filename) 
	{
		std::vector<std::string> lines;
		std::ifstream file(filename);

		if (!file) 
		{
			std::cerr << "Unable to open file: " << filename << std::endl;
			return lines;
		}

		std::string line;
		while (std::getline(file, line)) 
		{
			lines.push_back(line);
		}

		file.close();
		return lines;
	}

	static std::vector<std::string> string_split(const std::string &str, char delimiter, size_t limit) 
	{
		std::vector<std::string> parts;
		size_t start = 0;
		size_t end = str.find(delimiter);

		while (end != std::string::npos && parts.size() < limit - 1) 
		{
			parts.push_back(str.substr(start, end - start)); // Extract part
			start = end + 1; // Update start to the position after the delimiter
			end = str.find(delimiter, start); // Find the next delimiter
		}

		// Add the last part (or remaining substring)
		parts.push_back(str.substr(start));

		return parts;
	}

	bool ini_reader::field::try_get_uint16(uint16_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool ini_reader::field::try_get_int16(int16_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool ini_reader::field::try_get_uint32(uint32_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool ini_reader::field::try_get_int32(int32_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool ini_reader::field::try_get_uint64(uint64_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool ini_reader::field::try_get_int64(int64_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool ini_reader::field::try_get_boolean(bool &b)
	{
        std::string lowercaseValue = value;
        
        for (auto &ch : lowercaseValue) 
		{
            ch = std::tolower(ch);
        }

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
		
		if (!std::filesystem::exists(filePath))
			throw std::runtime_error("The given file path does not exist: " + filePath);

		std::vector<std::string> lines = read_all_lines(filePath);

		if (lines.size() == 0)
			throw std::runtime_error("The given file has no content");

		std::unordered_map<std::string, field> fields;

		for (size_t i = 0; i < lines.size(); i++)
		{
			lines[i] = strip_comment(lines[i]);

			if (lines[i].size() == 0)
				continue;

			auto parts = string_split(lines[i], ' ', 2); // Split on the first space only

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