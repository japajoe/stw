#ifndef STW_ARG_PARSER_HPP
#define STW_ARG_PARSER_HPP

#include <string_view>
#include <vector>
#include <optional>

namespace stw
{
	class arg_parser
	{
	public:
		arg_parser(int argc, char **argv);
		bool has_flag(std::string_view flag) const;
		std::optional<std::string_view> get_option(std::string_view flag) const;
		// Returns arguments that don't start with '-' and aren't values of options
		std::vector<std::string_view> get_positional(const std::vector<std::string_view> &flagsWithValues) const;
	private:
		std::vector<std::string_view> args;
	};
}

#endif