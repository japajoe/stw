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

#ifndef STW_STRINGSTREAM_HPP
#define STW_STRINGSTREAM_HPP

#include <string>
#include <cstdint>
#include <charconv>
#include <system_error>
#include <string_view>
#include <type_traits>

namespace stw
{
	class stringstream
	{
	public:
		explicit stringstream(std::string &str) : target(str) {}

		inline stringstream &append(std::string_view value)
		{
			target.append(value);
			return *this;
		}

		// Use std::enable_if to ensure this template ONLY matches integers
		template <typename T, typename std::enable_if<std::is_integral_v<T>, int>::type = 0>
		inline stringstream &append_int(T value)
		{
			// std::to_chars doesn't support bool, and it's technically an integral type
			if constexpr (std::is_same_v<T, bool>)
			{
				target.append(value ? "true" : "false");
			}
			else
			{
				auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), value);
				if (ec == std::errc())
				{
					target.append(buffer, ptr - buffer);
				}
			}
			return *this;
		}

		inline stringstream& clear() 
		{
			target.clear();
			return *this;
		}

		inline stringstream &operator<<(std::string_view value)
		{
			return append(value);
		}

		template <typename T, typename std::enable_if<std::is_integral_v<T>, int>::type = 0>
		inline stringstream &operator<<(T value)
		{
			return append_int(value);
		}

	private:
		std::string &target;
		char buffer[32];
	};
}

#endif