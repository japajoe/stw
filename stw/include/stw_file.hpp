#ifndef STW_FILE_HPP
#define STW_FILE_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

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

namespace stw
{
	struct file_info
	{
		std::vector<uint8_t> data;
		uint64_t lastModified;
	};

	class file_cache
	{
	public:
		bool read_file(const std::string &filePath, uint8_t **pData, uint64_t *size);
		void clear();
	private:
		std::unordered_map<std::string,file_info> files;
	};
}

#endif