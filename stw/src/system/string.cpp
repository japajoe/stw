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
#include <unordered_map>
#include <iostream>
#include <charconv>
#include <array>

namespace stw::string
{
	struct html_entity_entry 
	{
		std::string_view name;
		std::string_view value;

		bool operator<(std::string_view search_name) const 
		{
			return name < search_name;
		}
	};

	// A sorted list of HTML entities. 
	// IMPORTANT: This list MUST be kept in alphabetical order for binary search to work.
	// See method 'create_html_entity_array' to generate a sorted array
	static constexpr std::array<html_entity_entry, 182> kEntityTable = {{
		{"Acirc", "Â"}, {"Agrave", "À"}, {"Alpha", "Α"}, {"Atilde", "Ã"}, 
		{"Auml", "Ä"}, {"Beta", "Β"}, {"Ccedil", "Ç"}, {"Chi", "Χ"}, 
		{"Dagger", "‡"}, {"Delta", "Δ"}, {"Eacute", "É"}, {"Ecirc", "Ê"}, 
		{"Epsilon", "Ε"}, {"Eta", "Η"}, {"Evert", "È"}, {"Gamma", "Γ"}, 
		{"Icirc", "Î"}, {"Igrave", "Ì"}, {"Iota", "Ι"}, {"Kappa", "Κ"}, 
		{"Lambda", "Λ"}, {"Mu", "Μ"}, {"Ntilde", "Ñ"}, {"Nu", "Ν"}, 
		{"Ocirc", "Ô"}, {"Ograve", "Ò"}, {"Omega", "Ω"}, {"Omicron", "Ο"}, 
		{"Otilde", "Õ"}, {"Ouml", "Ö"}, {"Phi", "Φ"}, {"Pi", "Π"}, 
		{"Prime", "″"}, {"Psi", "Ψ"}, {"Rho", "Ρ"}, {"Sigma", "Σ"}, 
		{"Tau", "Τ"}, {"Theta", "Θ"}, {"Ucirc", "Û"}, {"Ugrave", "Ù"}, 
		{"Upsilon", "Υ"}, {"Uuml", "Ü"}, {"Xi", "Ξ"}, {"Zeta", "Ζ"}, 
		{"acirc", "â"}, {"agrave", "à"}, {"alefsym", "ℵ"}, {"alpha", "α"}, 
		{"amp", "&"}, {"and", "∧"}, {"ang", "∠"}, {"apos", "'"}, 
		{"asymp", "≈"}, {"atilde", "ã"}, {"auml", "ä"}, {"bdquo", "„"}, 
		{"beta", "β"}, {"bull", "•"}, {"cap", "∩"}, {"ccedil", "ç"}, 
		{"cent", "¢"}, {"chi", "χ"}, {"clubs", "♣"}, {"cong", "≅"}, 
		{"copy", "©"}, {"crarr", "↵"}, {"cup", "∪"}, {"dArr", "⇓"}, 
		{"dagger", "†"}, {"darr", "↓"}, {"deg", "°"}, {"delta", "δ"}, 
		{"diams", "♦"}, {"eacute", "é"}, {"ecirc", "ê"}, {"empty", "∅"}, 
		{"epsilon", "ε"}, {"equiv", "≡"}, {"eta", "η"}, {"euro", "€"}, 
		{"evert", "è"}, {"exists", "∃"}, {"forall", "∀"}, {"gamma", "γ"}, 
		{"ge", "≥"}, {"gt", ">"}, {"hArr", "⇔"}, {"harr", "↔"}, 
		{"hearts", "♥"}, {"hellip", "…"}, {"icirc", "î"}, {"igrave", "ì"}, 
		{"image", "ℑ"}, {"infin", "∞"}, {"int", "∫"}, {"iota", "ι"}, 
		{"isin", "∈"}, {"kappa", "κ"}, {"lArr", "⇐"}, {"lambda", "λ"}, 
		{"larr", "←"}, {"lceil", "⌈"}, {"ldquo", "“"}, {"le", "≤"}, 
		{"lfloor", "⌊"}, {"lowast", "∗"}, {"loz", "◊"}, {"lsaquo", "‹"}, 
		{"lsquo", "‘"}, {"lt", "<"}, {"mdash", "—"}, {"middot", "·"}, 
		{"minus", "−"}, {"mu", "μ"}, {"nabla", "∇"}, {"nbsp", "\xc2\xa0"}, 
		{"ndash", "–"}, {"ne", "≠"}, {"ni", "∋"}, {"notin", "∉"}, 
		{"nsub", "⊄"}, {"ntilde", "ñ"}, {"nu", "ν"}, {"ocirc", "ô"}, 
		{"ograve", "ò"}, {"omega", "ω"}, {"omicron", "ο"}, {"oplus", "⊕"}, 
		{"or", "∨"}, {"otilde", "õ"}, {"otimes", "⊗"}, {"ouml", "ö"}, 
		{"para", "¶"}, {"part", "∂"}, {"permil", "‰"}, {"perp", "⊥"}, 
		{"phi", "φ"}, {"pi", "π"}, {"plusmn", "±"}, {"pound", "£"}, 
		{"prime", "′"}, {"prod", "∏"}, {"prop", "∝"}, {"psi", "ψ"}, 
		{"quot", "\""}, {"rArr", "⇒"}, {"radic", "√"}, {"rarr", "→"}, 
		{"rceil", "⌉"}, {"rdquo", "”"}, {"real", "ℜ"}, {"reg", "®"}, 
		{"rfloor", "⌋"}, {"rho", "ρ"}, {"rsaquo", "›"}, {"rsquo", "’"}, 
		{"sbquo", "‚"}, {"sdot", "⋅"}, {"sect", "§"}, {"sigma", "ς"}, 
		{"sim", "∼"}, {"spades", "♠"}, {"sub", "⊂"}, {"sube", "⊆"}, 
		{"sum", "∑"}, {"sup", "⊃"}, {"supe", "⊇"}, {"szlig", "ß"}, 
		{"tau", "τ"}, {"there4", "∴"}, {"theta", "θ"}, {"trade", "™"}, 
		{"uArr", "⇑"}, {"uarr", "↑"}, {"ucirc", "û"}, {"ugrave", "ù"}, 
		{"upsilon", "υ"}, {"uuml", "ü"}, {"weierp", "℘"}, {"xi", "ξ"}, 
		{"yen", "¥"}, {"zeta", "ζ"}
	}};

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

	std::string html_decode(std::string_view str) 
	{
		std::string result;
		result.reserve(str.size());

		for (size_t i = 0; i < str.size(); ++i) {
			if (str[i] == '&') {
				size_t end = str.find(';', i);
				if (end != std::string_view::npos && end - i > 2) {
					std::string_view entity = str.substr(i + 1, end - i - 1);

					// 1. Handle Numeric Entities
					if (entity[0] == '#') {
						uint32_t code = 0;
						std::errc ec{};
						
						if (entity.size() > 2 && (entity[1] == 'x' || entity[1] == 'X')) {
							auto sub = entity.substr(2);
							auto [ptr, error] = std::from_chars(sub.data(), sub.data() + sub.size(), code, 16);
							ec = error;
						} else {
							auto sub = entity.substr(1);
							auto [ptr, error] = std::from_chars(sub.data(), sub.data() + sub.size(), code);
							ec = error;
						}

						if (ec == std::errc{}) {
							// UTF-8 encoding logic
							if (code <= 0x7F) {
								result += static_cast<char>(code);
							} else if (code <= 0x7FF) {
								result += static_cast<char>(0xC0 | (code >> 6));
								result += static_cast<char>(0x80 | (code & 0x3F));
							} else if (code <= 0xFFFF) {
								result += static_cast<char>(0xE0 | (code >> 12));
								result += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
								result += static_cast<char>(0x80 | (code & 0x3F));
							} else if (code <= 0x10FFFF) {
								result += static_cast<char>(0xF0 | (code >> 18));
								result += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
								result += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
								result += static_cast<char>(0x80 | (code & 0x3F));
							}
							i = end; 
							continue;
						}
					} 
					// 2. Handle Named Entities
					else {
						// Custom comparator for std::lower_bound
						auto it = std::lower_bound(kEntityTable.begin(), kEntityTable.end(), entity,
							[](const html_entity_entry& entry, std::string_view val) {
								return entry.name < val;
							});

						if (it != kEntityTable.end() && it->name == entity) {
							result += it->value;
							i = end; 
							continue;
						}
					}
				}
			}
			result += str[i];
		}
		return result;
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

	std::string create_html_entity_array(std::string &data) 
	{
		// Raw input (can be expanded with more HTML5 entities)
		// std::string data = "quot=\" amp=& apos=' lt=< gt=> nbsp=  copy=© reg=® trade=™ euro=€ "
		// 				"cent=¢ pound=£ yen=¥ sect=§ deg=° plusmn=± para=¶ middot=· "
		// 				"ndash=– mdash=— lsquo=‘ rsquo=’ sbquo=‚ ldquo=“ rdquo=” bdquo=„ "
		// 				"dagger=† Dagger=‡ bull=• hellip=… permil=‰ prime=′ Prime=″ "
		// 				"lsaquo=‹ rsaquo=› ouml=ö auml=ä uuml=ü Ouml=Ö Auml=Ä Uuml=Ü "
		// 				"szlig=ß ccedil=ç Ccedil=Ç eacute=é Eacute=É agrave=à Agrave=À";

		std::vector<html_entity_entry> entities;
		std::stringstream ss(data);
		std::string pair;

		while (ss >> pair) {
			size_t pos = pair.find('=');
			if (pos != std::string::npos) {
				entities.push_back({pair.substr(0, pos), pair.substr(pos + 1)});
			}
		}

		// Sort alphabetically for binary search
		std::sort(entities.begin(), entities.end(), [](const html_entity_entry& a, const html_entity_entry& b) {
			return a.name < b.name;
		});

		// Remove potential duplicates
		entities.erase(std::unique(entities.begin(), entities.end(), [](const html_entity_entry& a, const html_entity_entry& b){
			return a.name == b.name;
		}), entities.end());

		auto escape_value = [](std::string_view v) -> std::string {
			if (v == "\"") return "\\\"";
			if (v == "\\") return "\\\\";
			return std::string(v);
		};

		std::stringstream output;
		size_t entriesPerLine = 4;

		// Code generation
		output << "static constexpr std::array<html_entity_entry, " << entities.size() << "> kEntityTable = {{\n";
		for (size_t i = 0; i < entities.size(); ++i) {
			if (i % entriesPerLine == 0) 
				output << "    ";
			
			output << "{\"" << entities[i].name << "\", \"" << escape_value(entities[i].value) << "\"}";
			
			if (i != entities.size() - 1) 
			{
				output << ", ";
				if ((i + 1) % entriesPerLine == 0) 
					output << "\n";
			}
		}

		output << "\n}};\n";

		return output.str();
	}
}