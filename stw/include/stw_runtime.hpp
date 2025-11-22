#ifndef STW_RUNTIME_HPP
#define STW_RUNTIME_HPP

#include <string>

namespace stw::runtime
{
	void *load_library(const std::string &filePath);
	void unload_library(void *libraryHandle);
	void *get_symbol(void *libraryHandle, const std::string &symbolName);
	bool find_library_path(const std::string &libraryName, std::string &libraryPath);
	void set_current_working_directory(const std::string &directoryPath);
	std::string get_current_working_directory();
}

#endif