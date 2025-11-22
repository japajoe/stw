#ifndef STW_DIRECTORY_HPP
#define STW_DIRECTORY_HPP

#include <string>
#include <vector>

namespace stw::directory
{
	bool exists(const std::string &directoryPath);
	void create(const std::string &directoryPath);
	std::vector<std::string> get_files(const std::string &directoryPath, bool recursive = false);
}

#endif