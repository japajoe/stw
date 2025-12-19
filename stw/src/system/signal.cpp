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

#include "signal.hpp"
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