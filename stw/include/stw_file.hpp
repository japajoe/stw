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
	std::vector<std::string> read_all_lines(const std::string &filePath);
	size_t get_size(const std::string &filePath);
	bool is_within_directory(const std::string &filePath, const std::string &directoryPath);
	std::string get_name(const std::string &filePath, bool withExtension);
	std::string get_extension(const std::string &filePath);
}

#endif