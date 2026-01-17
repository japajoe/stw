#include "arg_parser.hpp"
#include <iostream>
#include <cassert>
#include <algorithm>

namespace stw
{
	arg_parser::arg_parser(int argc, char **argv)
	{
		for (int i = 1; i < argc; ++i)
		{
			args.emplace_back(argv[i]);
		}
	}

	bool arg_parser::has_flag(std::string_view flag) const
	{
		for (const auto &arg : args)
		{
			if (arg == flag)
				return true;
		}
		
		return false;
	}

	std::optional<std::string_view> arg_parser::get_option(std::string_view flag) const
	{
		for (size_t i = 0; i < args.size(); ++i)
		{
			if (args[i] == flag && (i + 1) < args.size())
			{
				// Ensure the next item isn't another flag
				if (args[i + 1].empty() || args[i + 1][0] != '-')
				{
					return args[i + 1];
				}
			}
		}

		return std::nullopt;
	}

	std::vector<std::string_view> arg_parser::get_positional(const std::vector<std::string_view> &flagsWithValues) const
	{
		std::vector<std::string_view> positionals;

		for (size_t i = 0; i < args.size(); ++i)
		{
			// Skip if it's a flag (starts with -)
			if (args[i][0] == '-')
				continue;

			// Skip if it's a value belonging to the previous flag
			if (i > 0 && args[i - 1][0] == '-')
			{
				bool prevWasValueOption = std::find(flagsWithValues.begin(),
													flagsWithValues.end(),
													args[i - 1]) != flagsWithValues.end();
				if (prevWasValueOption)
					continue;
			}

			positionals.push_back(args[i]);
		}

		return positionals;
	}
}