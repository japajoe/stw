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

#ifndef STW_RUNTIME_HPP
#define STW_RUNTIME_HPP

#include <string>
#include <vector>

namespace stw::runtime
{
	void *load_library(const std::string &filePath);
	void unload_library(void *libraryHandle);
	void *get_symbol(void *libraryHandle, const std::string &symbolName);
	bool find_library_path(const std::string &libraryName, std::string &libraryPath);
	void set_current_working_directory(const std::string &directoryPath);
	std::string get_current_working_directory();
	bool run_command(const std::string &cmd, const std::vector<std::string> &args, std::string &output);
}

#endif