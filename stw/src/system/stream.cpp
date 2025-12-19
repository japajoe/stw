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

#include "stream.hpp"
#include <stdexcept>
#include <cstring>

namespace stw
{
	// Helper function to translate file_access to open mode
	static std::ios_base::openmode access_to_openmode(file_access access)
	{
		switch (access)
		{
		case file_access_read:
			return std::ios::in;
		case file_access_write:
			return std::ios::out | std::ios::trunc;  // Truncate the file if it exists
		case file_access_read_write:
			return std::ios::in | std::ios::out;
		default:
			throw std::invalid_argument("Invalid file access type");
		}
	}

	file_stream::file_stream(const std::string &filePath, file_access access)
	{
		this->access = access;
		std::ios_base::openmode mode = access_to_openmode(access);
		file.open(filePath, mode);

		if (!file)
			throw std::runtime_error("Failed to open file: " + filePath);

		std::streampos currentPos = file.tellg();
		file.seekg(0, std::ios::end);
		length = static_cast<int64_t>(file.tellg());
		file.seekg(currentPos);
	}

	int64_t file_stream::read(void *buffer, size_t size)
	{
		if (!(access == file_access_read || access == file_access_read_write))
			throw std::runtime_error("File not opened in read mode");

		file.read(reinterpret_cast<char*>(buffer), size);
		int64_t bytesRead = file.gcount();
		readPosition += bytesRead;
		return bytesRead;
	}

	int64_t file_stream::write(const void *buffer, size_t size)
	{
		if (!(access == file_access_write || access == file_access_read_write))
			throw std::runtime_error("File not opened in write mode");

		file.write(reinterpret_cast<const char*>(buffer), size);
		if (!file)
			throw std::runtime_error("Failed to write to file");

		writePosition += size;
		return size;
	}

	int64_t file_stream::seek(int64_t offset, seek_origin origin)
	{
		if (origin == seek_origin_begin)
		{
			file.seekg(offset, std::ios::beg);
			file.seekp(offset, std::ios::beg);
		}
		else if (origin == seek_origin_current)
		{
			file.seekg(offset, std::ios::cur);
			file.seekp(offset, std::ios::cur);
		}
		else if (origin == seek_origin_end)
		{
			file.seekg(offset, std::ios::end);
			file.seekp(offset, std::ios::end);
		}
		else
		{
			throw std::invalid_argument("Invalid seek origin");
		}

		if (!file)
			throw std::runtime_error("Seek operation failed");

		readPosition = file.tellg();
		writePosition = file.tellp();

		if(access == file_access_read)
			return readPosition;
		else if(access == file_access_write)
			return writePosition;
		else
			return readPosition;
	}

	file_stream::~file_stream()
	{
		if (file.is_open())
			file.close();
	}

	memory_stream::memory_stream(void *memory, size_t size)
	{
		this->memory = memory;
		this->size = size;
		length = size;
	}

	int64_t memory_stream::read(void *buffer, size_t bytesToRead)
	{
		if (!memory || !buffer)
			return 0;

		size_t available = (readPosition < (int64_t)size)
			? size - readPosition
			: 0;

		size_t toRead = (bytesToRead <= available) ? bytesToRead : available;

		std::memcpy(buffer, static_cast<uint8_t*>(memory) + readPosition, toRead);

		readPosition += toRead;
		return static_cast<int64_t>(toRead);
	}

	int64_t memory_stream::write(const void *buffer, size_t bytesToWrite)
	{
		if (!memory || !buffer)
			return 0;

		size_t available = (writePosition < (int64_t)size)
			? size - writePosition
			: 0;

		size_t toWrite = (bytesToWrite <= available) ? bytesToWrite : available;

		std::memcpy(static_cast<uint8_t*>(memory) + writePosition, buffer, toWrite);

		writePosition += toWrite;
		return static_cast<int64_t>(toWrite);
	}

	int64_t memory_stream::seek(int64_t offset, seek_origin origin)
	{
		int64_t newPos = 0;

		switch (origin)
		{
		case seek_origin_begin:
			newPos = offset;
			break;

		case seek_origin_current:
			newPos = readPosition + offset; // use readPosition as unified cursor
			break;

		case seek_origin_end:
			newPos = static_cast<int64_t>(size) + offset;
			break;

		default:
			throw std::invalid_argument("Invalid seek origin");
		}

		// Clamp to valid range
		if (newPos < 0) newPos = 0;
		if (newPos > (int64_t)size) newPos = size;

		// Sync both read and write cursors
		readPosition = writePosition = newPos;

		return newPos;
	}
}