#ifndef STW_PLATFORM_HPP
#define STW_PLATFORM_HPP

#if defined(_WIN32)
	#define STW_PLATFORM_WINDOWS
#endif

#if defined(__linux__) || defined(__FreeBSD__)
	#define STW_PLATFORM_LINUX
#endif

#if defined(__APPLE__)
	#define STW_PLATFORM_MAC
#endif

#endif