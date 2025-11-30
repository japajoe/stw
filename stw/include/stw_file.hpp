#ifndef STW_FILE_HPP
#define STW_FILE_HPP

#include <string>
#include <vector>
#include <cstdint>

namespace stw::file
{
	bool exists(const std::string &filePath);
	void write_all_text(const std::string &filePath, const std::string &text);
	void write_all_bytes(const std::string &filePath, const void *data, size_t size);
	std::string read_all_text(const std::string &filePath);
	std::vector<uint8_t> read_all_bytes(const std::string &filePath);
	size_t get_size(const std::string &filePath);
	bool is_within_directory(const std::string &filePath, const std::string &directoryPath);
	std::string get_name(const std::string &filePath, bool withExtension);
	std::string get_extension(const std::string &filePath);
}

#endif