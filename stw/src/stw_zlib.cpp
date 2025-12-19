#include "stw_zlib.hpp"
#include "stw_runtime.hpp"
#include "stw_platform.hpp"
#include <stdexcept>
//#include <zlib.h>

namespace stw::zlib
{
	#ifdef SYS16BIT
	#  if defined(M_I86SM) || defined(M_I86MM)
		/* MSC small or medium model */
	#    define SMALL_MEDIUM
	#    ifdef _MSC_VER
	#      define FAR _far
	#    else
	#      define FAR far
	#    endif
	#  endif
	#  if (defined(__SMALL__) || defined(__MEDIUM__))
		/* Turbo C small or medium model */
	#    define SMALL_MEDIUM
	#    ifdef __BORLANDC__
	#      define FAR _far
	#    else
	#      define FAR far
	#    endif
	#  endif
	#endif

	#ifndef FAR
	#  define FAR
	#endif

	typedef unsigned char Byte;
	typedef unsigned long uLong;
	typedef uLong FAR uLongf;

	#ifdef SMALL_MEDIUM
	/* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
	#  define Bytef Byte FAR
	#else
	typedef Byte  FAR Bytef;
	#endif

	#define Z_OK 0

	typedef uLong (*compressBound_t)(uLong sourceLen);
	typedef int (*compress_t)(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);

	static compressBound_t compressBound = nullptr;
	static compress_t compress = nullptr;
	static void *libraryHandle = nullptr;

	static bool is_loaded()
	{
		return libraryHandle != nullptr;
	}

	static bool is_initialized(void *fn, const std::string &name)
	{
		if(fn)
			return true;
		
		fprintf(stderr, "Failed to loaded function: %s\n", name.c_str());
		if(libraryHandle)
			stw::runtime::unload_library(libraryHandle);
		libraryHandle = nullptr;
		return false;
	}

	static bool load_library(const std::string &libraryPath)
	{
		if(libraryHandle)
			return true;

		libraryHandle = stw::runtime::load_library(libraryPath);

		if(!libraryHandle)
			return false;

		compressBound = (compressBound_t)stw::runtime::get_symbol(libraryHandle, "compressBound");
		compress = (compress_t)stw::runtime::get_symbol(libraryHandle, "compress");

		if(!is_initialized(&compressBound, "compressBound"))
			return false;
		if(!is_initialized(&compress, "compress"))
			return false;

		return true;
	}

	static void close_library()
	{
		if(libraryHandle)
			stw::runtime::unload_library(libraryHandle);
		libraryHandle = nullptr;
	}

	static bool load_zlib()
	{
		if(is_loaded())
			return true;
	
		std::string libraryPath;
	#if defined(STW_PLATFORM_WINDOWS)
		libraryPath = "libz.dll";
	#elif defined(STW_PLATFORM_LINUX)
		stw::runtime::find_library_path("libz.so", libraryPath);
	#elif defined(STW_PLATFORM_MAC)
		//Not implemented yet
		return false;
	#endif
	
		if(libraryPath.size() > 0)
		{
			if(load_library(libraryPath))
				return true;
		}

		return false;
	}

	//deflate algorithm
	bool compress_data(const std::string &data, std::string &compressedData)
	{
		if(!load_zlib())
			return false;

		uLongf compressedSize = compressBound(data.size());
		compressedData = std::string(compressedSize, '\0');

		if (compress(reinterpret_cast<Bytef*>(&compressedData[0]), &compressedSize, 
		 			reinterpret_cast<const Bytef*>(data.data()), data.size()) != Z_OK) {
		 	throw std::runtime_error("Failed to compress data");
		}
		
		compressedData.resize(compressedSize); // Resize to actual compressed size
		return compressedData.size() > 0;
	}
}
