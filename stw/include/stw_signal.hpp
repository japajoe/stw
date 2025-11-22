#ifndef STW_SIGNAL_HPP
#define STW_SIGNAL_HPP

#include <functional>
#include <csignal>

namespace stw::signal
{
	using signal_handler = std::function<void(int num)>;

	void register_handler(signal_handler callback);
	void register_signal(int num);
}

#endif