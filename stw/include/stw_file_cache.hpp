#ifndef STW_FILE_CACHE_HPP
#define STW_FILE_CACHE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace stw
{
	class file_cache
	{
	public:
		file_cache();
		bool read_file(const std::string &filePath, uint8_t **pData, uint64_t *size);
		void clear();
		void invalidate();
		void set_max_age(uint64_t maxAgeInSeconds);
		uint64_t get_max_age() const;
		struct file_info
		{
			std::string path;
			std::vector<uint8_t> data;
			uint64_t lastModified;
			uint64_t lastAccessed;
		};
	private:
		std::unordered_map<std::string,file_info> files;
		uint64_t maxAge;
	};
}

#endif