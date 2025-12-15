# stw
Small library for making web based applications.

# Features
- HTTP client.
- HTTP server.
- Web socket implementation for client and server.

# Requirements for Linux
- libcurl (libcurl.so)
- libssl (libssl.so)
- libcrypto (libcrypto.so)

# Requirements for Windows
- libcurl (libcurl-x64.dll)
- libssl (libssl-3-x64.dll)
- libcrypto (libcrypto-3-x64.dll)

No linking is required with any libraries, but on Linux you must have these libraries somewhere in your system path. On Windows you must place the dll files in the same directory as the executable. MacOS is not fully supported (yet).

# Disclaimer
This library is just for educational purposes.
