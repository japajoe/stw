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

#include "stw_file.hpp"
#include "stw_date_time.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace stw::file
{
    static const char DirectorySeparatorChar = '\\';
    static const char AltDirectorySeparatorChar = '/';
    static const char VolumeSeparatorChar = ':';

	bool exists(const std::string &filePath)
	{
		return fs::exists(filePath) && fs::is_regular_file(filePath);
	}

	void write_all_text(const std::string &filePath, const std::string &text)
	{
        std::ofstream f(filePath, std::ios_base::out);
        if (f.is_open())
        {
            f << text;
            f.close();
        }
	}

	void write_all_bytes(const std::string &filePath, const void *data, size_t size)
	{
        std::ofstream f(filePath, std::ios::binary);

        if (f)
            f.write(reinterpret_cast<const char*>(data), size);

        if (f.is_open())
            f.close();
	}

	std::string read_all_text(const std::string &filePath)
	{
		std::ifstream f(filePath);
		
        if (!f.is_open())
			throw std::runtime_error("Could not open file: " + filePath);

		std::ostringstream contentStream;
		contentStream << f.rdbuf();
        f.close();
		return contentStream.str();
	}

	std::vector<uint8_t> read_all_bytes(const std::string &filePath)
	{
		std::ifstream f(filePath, std::ios::binary);
		
        if (!f.is_open()) 
			throw std::runtime_error("Could not open file: " + filePath);

		f.seekg(0, std::ios::end);
		std::streamsize fileSize = f.tellg();
		f.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(fileSize);

		if (!f.read(reinterpret_cast<char*>(buffer.data()), fileSize))
			throw std::runtime_error("Error reading file: " + filePath);
        
        f.close();
		return buffer;
	}

	std::vector<std::string> read_all_lines(const std::string &filePath)
	{
		std::ifstream f(filePath, std::ios::binary);
		
        if (!f.is_open()) 
			throw std::runtime_error("Unable to open file: " + filePath);

		std::vector<std::string> lines;
		std::string line;

		while (std::getline(f, line)) 
		{
			lines.push_back(line);
		}

		f.close();
		return lines;
	}

	size_t get_size(const std::string &filePath)
	{
        if(!exists(filePath))
            return 0;
		return fs::file_size(fs::path(filePath));
	}

    bool is_within_directory(const std::string &filePath, const std::string &directoryPath)
    {
        if(!exists(filePath))
            return false;

        fs::path directoryPath_ = fs::absolute(directoryPath);
        fs::path filePath_ = fs::absolute(filePath);

        auto const normRoot = fs::canonical(directoryPath_);
        auto const normChild = fs::canonical(filePath_);
        
        auto itr = std::search(normChild.begin(), normChild.end(), 
                            normRoot.begin(), normRoot.end());
        
        return itr == normChild.begin();
    }

    std::string get_name(const std::string &filePath, bool withExtension)
    {
        if (filePath.size() > 0)
        {
            size_t length = filePath.size();

            for (size_t i = length; --i > 0;)
            {
                char ch = filePath[i];

                if (ch == DirectorySeparatorChar || ch == AltDirectorySeparatorChar || ch == VolumeSeparatorChar)
                {
                    if(withExtension)
                    {
                        return filePath.substr(i + 1, length - i - 1);
                    }
                    else
                    {
                        // Find the position of the last dot (.)
                        size_t dotPos = filePath.find_last_of('.');
                        if (dotPos != std::string::npos && dotPos > i)
                            return filePath.substr(i + 1, dotPos - i - 1);
                        else
                            return filePath.substr(i + 1, length - i - 1);
                    }
                }
            }
        }
        return filePath;
    }

    std::string get_extension(const std::string &filePath)
    {
        if (filePath.size() == 0)
            return "";

        size_t length = filePath.size();

        for (int i = length; --i > 0;)
        {
            char ch = filePath[i];

            if (ch == '.')
            {
                if (i != length - 1)
                    return filePath.substr(i, length - i);
                else
                    return "";
            }

            if (ch == DirectorySeparatorChar || ch == AltDirectorySeparatorChar || ch == VolumeSeparatorChar)
                break;
        }

        return "";
    }
}