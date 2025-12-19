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

#include "directory.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace stw::directory
{
	bool exists(const std::string &directoryPath)
	{
		fs::path path = fs::path(directoryPath);
        return fs::exists(path) && fs::is_directory(path);
	}

	bool create(const std::string &directoryPath)
	{
        if (!exists(directoryPath))
            return fs::create_directories(fs::path(directoryPath));
		return false;
	}

	std::vector<std::string> get_files(const std::string &directoryPath, bool recursive)
	{
		std::vector<std::string> files;

		if(!exists(directoryPath))
			return files;

        fs::path dirPath = fs::path(directoryPath);

        if (recursive)
        {
            for (const auto &entry : fs::recursive_directory_iterator(dirPath))
            {
                if (entry.is_regular_file())
                {
                    files.push_back(entry.path().string());
                }
            }
        }
        else
        {
            for (const auto &entry : fs::directory_iterator(dirPath))
            {
                if (entry.is_regular_file())
                {
                    files.push_back(entry.path().string());
                }
            }
        }

        return files;
	}
}