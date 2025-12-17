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

#ifndef STW_FILE_CACHE_HPP
#define STW_FILE_CACHE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <mutex>

namespace stw
{
	class file_cache
	{
	public:
		file_cache();
		bool read_file(const std::string &filePath, uint8_t **pData, uint64_t *size);
		void clear();
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
		mutable std::mutex mutex;
		uint64_t maxAge;
		void invalidate();
	};
}

#endif