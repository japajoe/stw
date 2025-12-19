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

#ifndef STW_BINARY_STREAM_HPP
#define STW_BINARY_STREAM_HPP

#include <string>
#include <cstdint>

namespace stw
{
	enum byte_order
	{
		byte_order_big_endian,
		byte_order_little_endian,
		byte_order_native
	};

	class binary_stream
	{
	public:
		binary_stream(uint8_t *buffer, uint32_t size);
		binary_stream(const binary_stream &other);
		binary_stream(binary_stream &&other) noexcept;
		binary_stream &operator=(const binary_stream &other);
		binary_stream &operator=(binary_stream &&other) noexcept;
		uint32_t get_buffer_size() const;
		uint32_t get_length() const;
		void set_read_pointer(uint32_t offset);
		uint32_t get_read_pointer() const;
		void set_write_pointer(uint32_t offset);
		uint32_t get_write_pointer() const;
		void reset();
		bool write_uint8(uint8_t value);
		bool write_int8(int8_t value);
		bool write_uint16(uint16_t value, byte_order byteOrder = byte_order_native);
		bool write_int16(int16_t value, byte_order byteOrder = byte_order_native);
		bool write_uint32(uint32_t value, byte_order byteOrder = byte_order_native);
		bool write_int32(int32_t value, byte_order byteOrder = byte_order_native);
		bool write_uint64(uint64_t value, byte_order byteOrder = byte_order_native);
		bool write_int64(int64_t value, byte_order byteOrder = byte_order_native);
		bool write_float(float value, byte_order byteOrder = byte_order_native);
		bool write_double(double value, byte_order byteOrder = byte_order_native);
		bool write_bytes(const uint8_t *value, size_t size);
		bool write_string(const std::string &value, size_t size);
		bool read_uint8(uint8_t &value);
		bool read_int8(int8_t &value);
		bool read_uint16(uint16_t &value, byte_order byteOrder = byte_order_native);
		bool read_int16(int16_t &value, byte_order byteOrder = byte_order_native);
		bool read_uint32(uint32_t &value, byte_order byteOrder = byte_order_native);
		bool read_int32(int32_t &value, byte_order byteOrder = byte_order_native);
		bool read_uint64(uint64_t &value, byte_order byteOrder = byte_order_native);
		bool read_int64(int64_t &value, byte_order byteOrder = byte_order_native);
		bool read_float(float &value, byte_order byteOrder = byte_order_native);
		bool read_double(double &value, byte_order byteOrder = byte_order_native);
		bool read_bytes(uint8_t *value, size_t size);
		bool read_string(std::string &value, size_t size);
	private:
		uint8_t *m_buffer;
		uint32_t m_bufferSize;
		uint32_t m_readPointer;
		uint32_t m_writePointer;
		bool write(const void *pData, uint32_t size);
		bool read(void *pData, uint32_t size);
		void convert_endianness(void *value, uint32_t size, byte_order byteOrder);
		bool is_little_endian() const;
	};
}

#endif