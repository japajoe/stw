#include "stw_directory.hpp"
#include <filesystem>

namespace fs = std::filesystem;

namespace stw::directory
{
	bool exists(const std::string &directoryPath)
	{
		fs::path path = fs::path(directoryPath);
        return fs::exists(path) && fs::is_directory(path);
	}

	void create(const std::string &directoryPath)
	{
        if (!exists(directoryPath))
            fs::create_directories(fs::path(directoryPath));
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