#include "stw_binary_stream.hpp"
#include <utility>
#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace stw
{
	binary_stream::binary_stream(uint8_t *buffer, uint32_t size)
	{
		m_buffer = buffer;
		m_bufferSize = size;
		m_readPointer = 0;
		m_writePointer = 0;
	}

	binary_stream::binary_stream(const binary_stream &other)
	{
		m_buffer = other.m_buffer;
		m_bufferSize = other.m_bufferSize;
		m_readPointer = other.m_readPointer;
		m_writePointer = other.m_writePointer;
	}

	binary_stream::binary_stream(binary_stream &&other) noexcept
	{
		m_buffer = std::exchange(other.m_buffer, nullptr);
		m_bufferSize = std::exchange(other.m_bufferSize, 0);
		m_readPointer = std::exchange(other.m_readPointer, 0);
		m_writePointer = std::exchange(other.m_writePointer, 0);
	}

	binary_stream &binary_stream::operator=(const binary_stream &other)
	{
		if (this != &other)
		{
			m_buffer = other.m_buffer;
			m_bufferSize = other.m_bufferSize;
			m_readPointer = other.m_readPointer;
			m_writePointer = other.m_writePointer;
		}
		return *this;
	}

	binary_stream &binary_stream::operator=(binary_stream &&other) noexcept
	{
		if (this != &other)
		{
			m_buffer = std::exchange(other.m_buffer, nullptr);
			m_bufferSize = std::exchange(other.m_bufferSize, 0);
			m_readPointer = std::exchange(other.m_readPointer, 0);
			m_writePointer = std::exchange(other.m_writePointer, 0);
		}
		return *this;
	}

	uint32_t binary_stream::get_buffer_size() const
	{
		return m_bufferSize;
	}

	uint32_t binary_stream::get_length() const
	{
		return m_writePointer;
	}

	void binary_stream::set_read_pointer(uint32_t offset)
	{
		if (offset >= m_bufferSize)
			throw std::out_of_range("Read pointer exceeds buffer size");
		m_readPointer = offset;
	}

	uint32_t binary_stream::get_read_pointer() const
	{
		return m_readPointer;
	}

	void binary_stream::set_write_pointer(uint32_t offset)
	{
		if (offset >= m_bufferSize)
			throw std::out_of_range("Write pointer exceeds buffer size");
		m_writePointer = offset;
	}

	void binary_stream::reset()
	{
		m_readPointer = 0;
		m_writePointer = 0;
	}

	uint32_t binary_stream::get_write_pointer() const
	{
		return m_writePointer;
	}

	bool binary_stream::write(const void *pData, uint32_t size)
	{
		if (m_writePointer > m_bufferSize || m_writePointer + size > m_bufferSize)
			return false;

		std::memcpy(&m_buffer[m_writePointer], pData, size);
		m_writePointer += size;
		return true;
	}

	bool binary_stream::read(void *pData, uint32_t size)
	{
		if (m_readPointer > m_bufferSize || m_readPointer + size > m_bufferSize)
			return false;

		std::memcpy(pData, &m_buffer[m_readPointer], size);
		m_readPointer += size;
		return true;
	}

	void binary_stream::convert_endianness(void *value, uint32_t size, byte_order byteOrder)
	{
		if (byteOrder == byte_order_native)
			return; // No conversion needed

		if (byteOrder == byte_order_big_endian)
		{
			if (is_little_endian())
			{
				uint8_t *bytePtr = reinterpret_cast<uint8_t *>(value);
				std::reverse(bytePtr, bytePtr + size);
			}
		}
		else if (byteOrder == byte_order_little_endian)
		{
			if (!is_little_endian())
			{
				uint8_t *bytePtr = reinterpret_cast<uint8_t *>(value);
				std::reverse(bytePtr, bytePtr + size);
			}
		}
	}

	bool binary_stream::is_little_endian() const
	{
		const uint32_t num = 1;
		const uint8_t *bytePtr = reinterpret_cast<const uint8_t *>(&num);
		return bytePtr[0] == 1; // If the first byte is 1, it's little-endian
	}

	bool binary_stream::write_uint8(uint8_t value)
	{
		return write(&value, sizeof(uint8_t));
	}

	bool binary_stream::write_int8(int8_t value)
	{
		return write(&value, sizeof(int8_t));
	}

	bool binary_stream::write_uint16(uint16_t value, byte_order byteOrder)
	{
		convert_endianness(&value, sizeof(uint16_t), byteOrder);
		return write(&value, sizeof(uint16_t));
	}

	bool binary_stream::write_int16(int16_t value, byte_order byteOrder)
	{
		convert_endianness(&value, sizeof(int16_t), byteOrder);
		return write(&value, sizeof(int16_t));
	}

	bool binary_stream::write_uint32(uint32_t value, byte_order byteOrder)
	{
		convert_endianness(&value, sizeof(uint32_t), byteOrder);
		return write(&value, sizeof(uint32_t));
	}

	bool binary_stream::write_int32(int32_t value, byte_order byteOrder)
	{
		convert_endianness(&value, sizeof(int32_t), byteOrder);
		return write(&value, sizeof(int32_t));
	}

	bool binary_stream::write_uint64(uint64_t value, byte_order byteOrder)
	{
		convert_endianness(&value, sizeof(uint64_t), byteOrder);
		return write(&value, sizeof(uint64_t));
	}

	bool binary_stream::write_int64(int64_t value, byte_order byteOrder)
	{
		convert_endianness(&value, sizeof(int64_t), byteOrder);
		return write(&value, sizeof(int64_t));
	}

	bool binary_stream::write_float(float value, byte_order byteOrder)
	{
		convert_endianness(&value, sizeof(float), byteOrder);
		return write(&value, sizeof(float));
	}

	bool binary_stream::write_double(double value, byte_order byteOrder)
	{
		convert_endianness(&value, sizeof(double), byteOrder);
		return write(&value, sizeof(double));
	}

	bool binary_stream::write_bytes(const uint8_t *value, size_t size)
	{
		return write(&value, size);
	}

	bool binary_stream::write_string(const std::string &value, size_t size)
	{
		return write(value.data(), size);
	}

	bool binary_stream::read_uint8(uint8_t &value)
	{
		return read(&value, sizeof(uint8_t));
	}

	bool binary_stream::read_int8(int8_t &value)
	{
		return read(&value, sizeof(int8_t));
	}

	bool binary_stream::read_uint16(uint16_t &value, byte_order byteOrder)
	{
		if (!read(&value, sizeof(uint16_t)))
			return false;
		convert_endianness(&value, sizeof(uint16_t), byteOrder);
		return true;
	}

	bool binary_stream::read_int16(int16_t &value, byte_order byteOrder)
	{
		if (!read(&value, sizeof(int16_t)))
			return false;
		convert_endianness(&value, sizeof(int16_t), byteOrder);
		return true;
	}

	bool binary_stream::read_uint32(uint32_t &value, byte_order byteOrder)
	{
		if (!read(&value, sizeof(uint32_t)))
			return false;
		convert_endianness(&value, sizeof(uint32_t), byteOrder);
		return true;
	}

	bool binary_stream::read_int32(int32_t &value, byte_order byteOrder)
	{
		if (!read(&value, sizeof(int32_t)))
			return false;
		convert_endianness(&value, sizeof(int32_t), byteOrder);
		return true;
	}

	bool binary_stream::read_uint64(uint64_t &value, byte_order byteOrder)
	{
		if (!read(&value, sizeof(uint64_t)))
			return false;
		convert_endianness(&value, sizeof(uint64_t), byteOrder);
		return true;
	}

	bool binary_stream::read_int64(int64_t &value, byte_order byteOrder)
	{
		if (!read(&value, sizeof(int64_t)))
			return false;
		convert_endianness(&value, sizeof(int64_t), byteOrder);
		return true;
	}

	bool binary_stream::read_float(float &value, byte_order byteOrder)
	{
		if (!read(&value, sizeof(float)))
			return false;
		convert_endianness(&value, sizeof(float), byteOrder);
		return true;
	}

	bool binary_stream::read_double(double &value, byte_order byteOrder)
	{
		if (!read(&value, sizeof(double)))
			return false;
		convert_endianness(&value, sizeof(double), byteOrder);
		return true;
	}

	bool binary_stream::read_bytes(uint8_t *value, size_t size)
	{
		return read(value, size);
	}

	bool binary_stream::read_string(std::string &value, size_t size)
	{
		constexpr size_t MAX_BYTE_COUNT = 4;
		value.resize(size * MAX_BYTE_COUNT);
		return read(value.data(), size);
	}
}