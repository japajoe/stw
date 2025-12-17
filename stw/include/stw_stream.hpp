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

#ifndef STW_STREAM_HPP
#define STW_STREAM_HPP

#include <cstdlib>
#include <cstdint>
#include <string>
#include <fstream>

namespace stw
{
	enum seek_origin
	{
		seek_origin_begin,
		seek_origin_current,
		seek_origin_end
	};

	enum file_access
	{
		file_access_read,
		file_access_write,
		file_access_read_write
	};

	class stream
	{
	public:
		virtual ~stream() = default;
		virtual int64_t read(void *buffer, size_t size) = 0;
		virtual int64_t write(const void *buffer, size_t size) = 0;
		virtual int64_t seek(int64_t offset, seek_origin origin) = 0;
		int64_t get_length() const { return length; }
	protected:
		int64_t readPosition = 0;
		int64_t writePosition = 0;
		int64_t length= 0;
	};

	class file_stream : public stream
	{
	public:
		file_stream(const std::string &filePath, file_access access);
		~file_stream();
		int64_t read(void *buffer, size_t size) override;
		int64_t write(const void *buffer, size_t size) override;
		int64_t seek(int64_t offset, seek_origin origin) override;
	private:
		file_access access;
		std::fstream file;
	};

	class memory_stream : public stream
	{
	public:
		memory_stream(void *memory, size_t size);
		int64_t read(void *buffer, size_t size) override;
		int64_t write(const void *buffer, size_t size) override;
		int64_t seek(int64_t offset, seek_origin origin) override;
	private:
		void *memory;
		size_t size;
	};
}

#endif