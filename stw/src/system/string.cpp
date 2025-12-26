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

#include "string.hpp"
#include <sstream>
#include <algorithm>

namespace stw::string
{
    bool is_valid_utf8(const void *data, size_t size) 
	{
        int numBytes = 0; // Number of bytes expected in the current UTF-8 character
        unsigned char byte;
        const uint8_t *pData = reinterpret_cast<const uint8_t*>(data);

        for (size_t i = 0; i < size; ++i) 
		{
            byte = pData[i];

            if (numBytes == 0) 
			{
                // Determine the number of bytes in the UTF-8 character
                if ((byte & 0x80) == 0) 
				{
                    // 1-byte character (ASCII)
                    continue;
                } 
				else if ((byte & 0xE0) == 0xC0) 
				{
                    // 2-byte character
                    numBytes = 1;
                } 
				else if ((byte & 0xF0) == 0xE0) 
				{
                    // 3-byte character
                    numBytes = 2;
                } 
				else if ((byte & 0xF8) == 0xF0) 
				{
                    // 4-byte character
                    numBytes = 3;
                } 
				else 
				{
                    // Invalid first byte
                    return false;
                }
            } 
			else 
			{
                // Check continuation bytes
                if ((byte & 0xC0) != 0x80) 
				{
                    return false; // Invalid continuation byte
                }
                numBytes--;
            }
        }

        return numBytes == 0; // Ensure all characters were complete
    }

	std::string url_decode(const std::string &str) 
	{
		std::string ret;
		char ch;
		int i, ii;
		
		for (i = 0; i < str.length(); i++) 
		{
			if (str[i] == '%') 
			{
				// Check if there are at least two characters following '%'
				if (i + 2 < str.length()) 
				{
					// Convert the hex string to an integer
					std::istringstream hexStream(str.substr(i + 1, 2));
					if (hexStream >> std::hex >> ii) 
					{
						if (ii == 0) 
							throw std::runtime_error("Null byte detected");

						ch = static_cast<char>(ii);
						ret += ch;
						i += 2; // Skip the two hex digits
					} 
					else 
					{
						// Invalid hex sequence; treat as literal or throw error
						ret += str[i];
					}
				} 
				else 
				{
					// '%' at the end of string with no digits
					ret += str[i];
				}
			} 
			else if (str[i] == '+') 
			{
				// In URLs, '+' often represents a space
				ret += ' ';
			} 
			else 
			{
				ret += str[i];
			}
		}
		return ret;
	}

	bool compare(const std::string &str1, const std::string &str2, bool ignoreCase)
	{
		if(!ignoreCase)
			return str1 == str2;
			
		if (str1.length() != str2.length())
			return false;

		for (size_t i = 0; i < str1.length(); ++i) 
		{
			if (std::tolower(static_cast<unsigned char>(str1[i])) != std::tolower(static_cast<unsigned char>(str2[i])))
				return false;
		}
		return true; // All characters matched
	}

    bool contains(const std::string &haystack, const std::string &needle) 
	{
        return haystack.find(needle) != std::string::npos;
    }

    bool starts_with(const std::string &haystack, const std::string &needle)
    {
        if (haystack.length() >= needle.length()) 
            return (0 == haystack.compare(0, needle.length(), needle));
        return false;
    }

    bool ends_with(const std::string &haystack, const std::string &needle) 
	{
        if (haystack.length() >= needle.length()) 
            return (0 == haystack.compare(haystack.length() - needle.length(), needle.length(), needle));
        return false;
    }
    
    std::string trim(const std::string& str)
    {
        size_t start = 0;
        size_t end = str.length();

        // Find the first non-whitespace character
        while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) 
        {
            ++start;
        }

        // Find the last non-whitespace character
        while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) 
        {
            --end;
        }

        // Return the substring from the first to the last non-whitespace character
        return str.substr(start, end - start);
    }

    std::string trim_start(const std::string& str) 
    {
        size_t start = 0;

        // Find the first non-whitespace character
        while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) 
        {
            ++start;
        }

        // Return the substring from the first non-whitespace character to the end
        return str.substr(start);
    }

    std::string trim_end(const std::string& str)
    {
        size_t end = str.length();

        // Find the last non-whitespace character
        while (end > 0 && std::isspace(static_cast<unsigned char>(str[end - 1]))) 
        {
            --end;
        }

        // Return the substring from the beginning to the last non-whitespace character
        return str.substr(0, end);
    }

	std::vector<std::string> split(const std::string& str, char separator, size_t maxParts) 
    {
        std::vector<std::string> result;
        size_t start = 0;
        size_t end = 0;

        while ((end = str.find(separator, start)) != std::string::npos) 
        {
            result.push_back(str.substr(start, end - start));
            start = end + 1;

            if (maxParts > 0 && result.size() >= maxParts - 1) 
                break; // Stop if we have reached maximum parts
        }
        result.push_back(str.substr(start)); // Add the last part
        return result;
    }

    std::string replace(const std::string &str, const std::string &target, const std::string &replacement)
    {
        std::string newStr = str;
        size_t startPos = 0;

        while ((startPos = newStr.find(target, startPos)) != std::string::npos)
        {
            newStr.replace(startPos, target.length(), replacement);
            startPos += replacement.length();
        }

        return newStr;
    }

    std::string to_lower(const std::string &str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    std::string to_upper(const std::string &str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }

    std::string sub_string(const std::string &str, size_t startIndex)
    {
        return str.substr(startIndex);
    }

    std::string sub_string(const std::string &str, size_t startIndex, size_t length)
    {
        return str.substr(startIndex, length);
    }

    int64_t index_of(const std::string &str, const std::string &subStr)
    {
        size_t pos = str.find(subStr);
        if (pos != std::string::npos) 
            return static_cast<int64_t>(pos);
        return -1; // Indicates substring not found
    }

	bool try_parse_uint8(const std::string &value, uint8_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_uint16(const std::string &value, uint16_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_uint32(const std::string &value, uint32_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_uint64(const std::string &value, uint64_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_int8(const std::string &value, int8_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_int16(const std::string &value, int16_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_int32(const std::string &value, int32_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_int64(const std::string &value, int64_t &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_float(const std::string &value, float &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}

	bool try_parse_double(const std::string &value, double &v)
	{
        std::stringstream ss(value);
        ss >> v;

        if (ss.fail() || !ss.eof())
            return false;
        
        return true;
	}	
}