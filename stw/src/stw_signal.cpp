#include "stw_signal.hpp"
#include <iostream>
#include <cerrno>
#include <cstring>

namespace stw::signal
{
	static signal_handler gSignalHandler = nullptr;

	static void handle_signal(int signum)
	{
		if(gSignalHandler)
			gSignalHandler(signum);
	}

	void register_handler(signal_handler callback)
	{
		gSignalHandler = callback;
	}

	void register_signal(int num)
	{
		if (::signal(num, handle_signal) == SIG_ERR) 
		{
        	std::cerr << "Error setting up signal handler for signal " << num 
                  << ": " << strerror(errno) << '\n';
		}
	}
}