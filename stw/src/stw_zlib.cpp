#include "stw_zlib.hpp"
#include "stw_runtime.hpp"
#include "stw_platform.hpp"
#include <stdexcept>
//#include <zlib.h>

namespace stw::zlib
{
	#define ZLIB_VERSION "1.2.11"

	#define Z_NO_FLUSH      0
	#define Z_PARTIAL_FLUSH 1
	#define Z_SYNC_FLUSH    2
	#define Z_FULL_FLUSH    3
	#define Z_FINISH        4
	#define Z_BLOCK         5
	#define Z_TREES         6

	#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

	#define Z_NO_COMPRESSION         0
	#define Z_BEST_SPEED             1
	#define Z_BEST_COMPRESSION       9
	#define Z_DEFAULT_COMPRESSION  (-1)

	#define Z_DEFLATED   8

	#define Z_FILTERED            1
	#define Z_HUFFMAN_ONLY        2
	#define Z_RLE                 3
	#define Z_FIXED               4
	#define Z_DEFAULT_STRATEGY    0

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)

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

	#if defined(ZLIB_CONST) && !defined(z_const)
	#  define z_const const
	#else
	#  define z_const
	#endif

	typedef unsigned char Byte;
	typedef unsigned long uLong;
	typedef uLong FAR uLongf;
	typedef unsigned int uInt;  /* 16 bits or more */

	#ifdef SMALL_MEDIUM
	/* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
	#  define Bytef Byte FAR
	#else
	typedef Byte  FAR Bytef;
	#endif

	#ifdef __STDC_VERSION__
	#  ifndef STDC
	#    define STDC
	#  endif
	#  if __STDC_VERSION__ >= 199901L
	#    ifndef STDC99
	#      define STDC99
	#    endif
	#  endif
	#endif
	#if !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
	#  define STDC
	#endif
	#if !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
	#  define STDC
	#endif
	#if !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
	#  define STDC
	#endif
	#if !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
	#  define STDC
	#endif

	#ifdef STDC
	typedef void const *voidpc;
	typedef void FAR   *voidpf;
	typedef void       *voidp;
	#else
	typedef Byte const *voidpc;
	typedef Byte FAR   *voidpf;
	typedef Byte       *voidp;
	#endif



	typedef voidpf (*alloc_func) (voidpf opaque, uInt items, uInt size);
	typedef void   (*free_func)  (voidpf opaque, voidpf address);

	struct internal_state;

	typedef struct z_stream_s {
		z_const Bytef *next_in;     /* next input byte */
		uInt     avail_in;  /* number of bytes available at next_in */
		uLong    total_in;  /* total number of input bytes read so far */

		Bytef    *next_out; /* next output byte will go here */
		uInt     avail_out; /* remaining free space at next_out */
		uLong    total_out; /* total number of bytes output so far */

		z_const char *msg;  /* last error message, NULL if no error */
		struct internal_state FAR *state; /* not visible by applications */

		alloc_func zalloc;  /* used to allocate the internal state */
		free_func  zfree;   /* used to free the internal state */
		voidpf     opaque;  /* private data object passed to zalloc and zfree */

		int     data_type;  /* best guess about the data type: binary or text
							for deflate, or the decoding state for inflate */
		uLong   adler;      /* Adler-32 or CRC-32 value of the uncompressed data */
		uLong   reserved;   /* reserved for future use */
	} z_stream;

	typedef z_stream FAR *z_streamp;

	#define deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
          deflateInit2_((strm),(level),(method),(windowBits),(memLevel),\
                        (strategy), ZLIB_VERSION, (int)sizeof(z_stream))

	typedef uLong (*compressBound_t)(uLong sourceLen);
	typedef int (*compress_t)(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
	typedef int (*deflateInit2__t)(z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size);
	typedef int (*deflateEnd_t)(z_streamp strm);
	typedef int (*deflate_t)(z_streamp strm, int flush);

	static compressBound_t compressBound = nullptr;
	static compress_t compress = nullptr;
	static deflateInit2__t deflateInit2_ = nullptr;
	static deflateEnd_t deflateEnd = nullptr;
	static deflate_t deflate = nullptr;
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
		deflateInit2_ = (deflateInit2__t)stw::runtime::get_symbol(libraryHandle, "deflateInit2_");
		deflateEnd = (deflateEnd_t)stw::runtime::get_symbol(libraryHandle, "deflateEnd");
		deflate = (deflate_t)stw::runtime::get_symbol(libraryHandle, "deflate");

		if(!is_initialized(&compressBound, "compressBound"))
			return false;
		if(!is_initialized(&compress, "compress"))
			return false;
		if(!is_initialized(&deflateInit2_, "deflateInit2_"))
			return false;
		if(!is_initialized(&deflateEnd, "deflateEnd"))
			return false;
		if(!is_initialized(&deflate, "deflate"))
			return false;

		return true;
	}

	static void close_library()
	{
		if(libraryHandle)
			stw::runtime::unload_library(libraryHandle);
		libraryHandle = nullptr;
	}

	bool load_library()
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

	static bool compress_deflate(const std::string &data, std::string &compressedData)
	{
		uLongf compressedSize = compressBound(data.size());
		compressedData = std::string(compressedSize, '\0');

		if (compress(reinterpret_cast<Bytef*>(&compressedData[0]), &compressedSize, 
		 			reinterpret_cast<const Bytef*>(data.data()), data.size()) != Z_OK) 
		{
		 	return false;
		}
		
		compressedData.resize(compressedSize); // Resize to actual compressed size
		return compressedData.size() > 0;
	}

	static bool compress_gzip(const std::string &data, std::string &compressedData)
	{
		// Prepare the gzip stream
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;

		// Initialize the stream for compression
		if (deflateInit2(&strm, Z_BEST_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) != Z_OK)
			throw std::runtime_error("Failed to initialize compression");

		strm.avail_in = data.size();
		strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));

		// The output buffer
		size_t bufferSize = compressBound(data.size());
		compressedData = std::string(bufferSize, '\0');

		strm.avail_out = bufferSize;
		strm.next_out = reinterpret_cast<Bytef*>(&compressedData[0]);

		// Compress the data
		if (deflate(&strm, Z_FINISH) == Z_STREAM_ERROR) 
		{
			deflateEnd(&strm);
			throw std::runtime_error("Compression failed");
		}

		// Finalize the compression
		deflateEnd(&strm);

		// Resize the compressed data to the actual size
		compressedData.resize(bufferSize - strm.avail_out);

		return compressedData.size() > 0;
	}

	bool compress_data(const std::string &data, std::string &compressedData, compression_algorithm algorithm)
	{
		if(is_loaded())
			return false;

		switch (algorithm)
		{
		case compression_algorithm_deflate:
			return compress_deflate(data, compressedData);
		case compression_algorithm_gzip:
			return compress_gzip(data, compressedData);
		default:
			break;
		}
		return false;
	}
}
