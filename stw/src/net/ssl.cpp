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

#include "ssl.hpp"
#include "../core/platform.hpp"
#include "../system/runtime.hpp"
#include <stdexcept>
#include <utility>
#include <cstring>

namespace stw
{
	#define SSL_CTRL_SET_TLSEXT_HOSTNAME 55
	#define TLSEXT_NAMETYPE_host_name 0
	#define X509_FILETYPE_PEM 1
	#define SSL_FILETYPE_PEM X509_FILETYPE_PEM
	#define SSL_VERIFY_NONE 0x00
	#define SSL_VERIFY_PEER 0x01

	typedef struct ssl_method_st SSL_METHOD;
	typedef struct x509_store_ctx_st X509_STORE_CTX;
	typedef int (*SSL_verify_cb)(int preverify_ok, X509_STORE_CTX *x509_ctx);

	namespace openssl
	{
		static void *libraryHandleSsl = nullptr;

		typedef SSL_CTX *(*SSL_CTX_new_t)(const SSL_METHOD *meth);
		typedef const SSL_METHOD *(*TLS_method_t)(void);
		typedef const SSL_METHOD *(*TLS_server_method_t)(void);
		typedef SSL *(*SSL_new_t)(SSL_CTX *ctx);
		typedef int (*SSL_set_fd_t)(SSL *s, int fd);
		typedef long (*SSL_ctrl_t)(SSL *ssl, int cmd, long larg, void *parg);
		typedef int (*SSL_connect_t)(SSL *ssl);
		typedef int (*SSL_accept_t)(SSL *ssl);
		typedef int (*SSL_read_t)(SSL *ssl, void *buf, int num);
		typedef int (*SSL_write_t)(SSL *ssl, const void *buf, int num);
		typedef int (*SSL_peek_t)(SSL *ssl, void *buf, int num);
		typedef int (*SSL_shutdown_t)(SSL *s);
		typedef void (*SSL_free_t)(SSL *ssl);
		typedef void (*SSL_CTX_free_t)(SSL_CTX *ctx);
		typedef void (*SSL_CTX_set_verify_t)(SSL_CTX *ctx, int mode, SSL_verify_cb callback);
		typedef int (*SSL_CTX_use_certificate_file_t)(SSL_CTX *ctx, const char *file, int type);
		typedef int (*SSL_CTX_use_PrivateKey_file_t)(SSL_CTX *ctx, const char *file, int type);
		typedef int (*SSL_CTX_check_private_key_t)(const SSL_CTX *ctx);

		SSL_CTX_new_t SSL_CTX_new_ptr = nullptr;
		TLS_method_t TLS_method_ptr = nullptr;
		TLS_server_method_t TLS_server_method_ptr = nullptr;
		SSL_new_t SSL_new_ptr = nullptr;
		SSL_set_fd_t SSL_set_fd_ptr = nullptr;
		SSL_ctrl_t SSL_ctrl_ptr = nullptr;
		SSL_connect_t SSL_connect_ptr = nullptr;
		SSL_accept_t SSL_accept_ptr = nullptr;
		SSL_read_t SSL_read_ptr = nullptr;
		SSL_write_t SSL_write_ptr = nullptr;
		SSL_peek_t SSL_peek_ptr = nullptr;
		SSL_shutdown_t SSL_shutdown_ptr = nullptr;
		SSL_free_t SSL_free_ptr = nullptr;
		SSL_CTX_free_t SSL_CTX_free_ptr = nullptr;
		SSL_CTX_set_verify_t SSL_CTX_set_verify_ptr = nullptr;
		SSL_CTX_use_certificate_file_t SSL_CTX_use_certificate_file_ptr = nullptr;
		SSL_CTX_use_PrivateKey_file_t SSL_CTX_use_PrivateKey_file_ptr = nullptr;
		SSL_CTX_check_private_key_t SSL_CTX_check_private_key_ptr = nullptr;

		bool is_initialized(void *fn, const std::string &name)
		{
			if(fn)
				return true;
			
			fprintf(stderr, "Failed to loaded function: %s\n", name.c_str());
			if(libraryHandleSsl)
				stw::runtime::unload_library(libraryHandleSsl);
			libraryHandleSsl = nullptr;
			return false;
		}

		bool load_library(const std::string &libraryPathSsl)
		{
			if(libraryHandleSsl)
				return true;

			libraryHandleSsl = stw::runtime::load_library(libraryPathSsl);

			if(!libraryHandleSsl)
				return false;

			SSL_CTX_new_ptr = (SSL_CTX_new_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_CTX_new");
			TLS_method_ptr = (TLS_method_t)stw::runtime::get_symbol(libraryHandleSsl, "TLS_method");
			TLS_server_method_ptr = (TLS_server_method_t)stw::runtime::get_symbol(libraryHandleSsl, "TLS_server_method");
			SSL_new_ptr = (SSL_new_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_new");
			SSL_set_fd_ptr = (SSL_set_fd_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_set_fd");
			SSL_ctrl_ptr = (SSL_ctrl_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_ctrl");
			SSL_connect_ptr = (SSL_connect_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_connect");
			SSL_accept_ptr = (SSL_accept_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_accept");
			SSL_read_ptr = (SSL_read_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_read");
			SSL_write_ptr = (SSL_write_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_write");
			SSL_peek_ptr = (SSL_peek_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_peek");
			SSL_shutdown_ptr = (SSL_shutdown_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_shutdown");
			SSL_free_ptr = (SSL_free_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_free");
			SSL_CTX_free_ptr = (SSL_CTX_free_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_CTX_free");
			SSL_CTX_set_verify_ptr = (SSL_CTX_set_verify_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_CTX_set_verify");
			SSL_CTX_use_certificate_file_ptr = (SSL_CTX_use_certificate_file_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_CTX_use_certificate_file");
			SSL_CTX_use_PrivateKey_file_ptr = (SSL_CTX_use_PrivateKey_file_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_CTX_use_PrivateKey_file");
			SSL_CTX_check_private_key_ptr = (SSL_CTX_check_private_key_t)stw::runtime::get_symbol(libraryHandleSsl, "SSL_CTX_check_private_key");

			if(!is_initialized((void*)SSL_CTX_new_ptr, "SSL_CTX_new_ptr"))
				return false;
			if(!is_initialized((void*)TLS_method_ptr, "TLS_method_ptr"))
				return false;
			if(!is_initialized((void*)TLS_server_method_ptr, "TLS_server_method_ptr"))
				return false;
			if(!is_initialized((void*)SSL_new_ptr, "SSL_new_ptr"))
				return false;
			if(!is_initialized((void*)SSL_set_fd_ptr, "SSL_set_fd_ptr"))
				return false;
			if(!is_initialized((void*)SSL_ctrl_ptr, "SSL_ctrl_ptr"))
				return false;
			if(!is_initialized((void*)SSL_connect_ptr, "SSL_connect_ptr"))
				return false;
			if(!is_initialized((void*)SSL_accept_ptr, "SSL_accept_ptr"))
				return false;
			if(!is_initialized((void*)SSL_read_ptr, "SSL_read_ptr"))
				return false;
			if(!is_initialized((void*)SSL_write_ptr, "SSL_write_ptr"))
				return false;
			if(!is_initialized((void*)SSL_peek_ptr, "SSL_peek_ptr"))
				return false;
			if(!is_initialized((void*)SSL_shutdown_ptr, "SSL_shutdown_ptr"))
				return false;
			if(!is_initialized((void*)SSL_free_ptr, "SSL_free_ptr"))
				return false;
			if(!is_initialized((void*)SSL_CTX_free_ptr, "SSL_CTX_free_ptr"))
				return false;
			if(!is_initialized((void*)SSL_CTX_set_verify_ptr, "SSL_CTX_set_verify_ptr"))
				return false;
			if(!is_initialized((void*)SSL_CTX_use_certificate_file_ptr, "SSL_CTX_use_certificate_file_ptr"))
				return false;
			if(!is_initialized((void*)SSL_CTX_use_PrivateKey_file_ptr, "SSL_CTX_use_PrivateKey_file_ptr"))
				return false;
			if(!is_initialized((void*)SSL_CTX_check_private_key_ptr, "SSL_CTX_check_private_key_ptr"))
				return false;

			return true;
		}

		void close_library()
		{
			if(libraryHandleSsl)
				stw::runtime::unload_library(libraryHandleSsl);
			libraryHandleSsl = nullptr;
		}

		bool is_loaded()
		{
			return libraryHandleSsl != nullptr;
		}

		bool load_library()
		{
			if(is_loaded())
				return true;
		
			std::string sslPath;
		#if defined(STW_PLATFORM_WINDOWS)
			sslPath = "libssl-3-x64.dll";
		#elif defined(STW_PLATFORM_LINUX)
			stw::runtime::find_library_path("libssl.so", sslPath);
		#elif defined(STW_PLATFORM_MAC)
			//Not implemented yet
			return false;
		#endif
		
			if(sslPath.size() > 0)
			{
				if(openssl::load_library(sslPath))
					return true;
			}

			return false;
		}

		SSL_CTX *SSL_CTX_new(const SSL_METHOD *meth)
		{
			if(!libraryHandleSsl)
				return nullptr;
			return SSL_CTX_new_ptr(meth);
		}

		const SSL_METHOD *TLS_method()
		{
			if(!libraryHandleSsl)
				return nullptr;
			return TLS_method_ptr();
		}

		const SSL_METHOD *TLS_server_method()
		{
			if(!libraryHandleSsl)
				return nullptr;
			return TLS_server_method_ptr();
		}

		SSL *SSL_new(SSL_CTX *ctx)
		{
			if(!libraryHandleSsl)
				return nullptr;
			return SSL_new_ptr(ctx);
		}

		int SSL_set_fd(SSL *s, int fd)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_set_fd_ptr(s, fd);
		}

		long SSL_ctrl(SSL *ssl, int cmd, long larg, void *parg)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_ctrl_ptr(ssl, cmd, larg, parg);
		}

		int SSL_connect(SSL *ssl)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_connect_ptr(ssl);
		}

		int SSL_accept(SSL *ssl)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_accept_ptr(ssl);
		}

		int SSL_read(SSL *ssl, void *buf, int num)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_read_ptr(ssl, buf, num);
		}

		int SSL_write(SSL *ssl, const void *buf, int num)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_write_ptr(ssl, buf, num);
		}

		int SSL_peek(SSL *ssl, void *buf, int num)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_peek_ptr(ssl, buf, num);
		}

		int SSL_shutdown(SSL *s)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_shutdown_ptr(s);
		}

		void SSL_free(SSL *ssl)
		{
			if(!libraryHandleSsl)
				return;
			SSL_free_ptr(ssl);
		}

		void SSL_CTX_free(SSL_CTX *ctx)
		{
			if(!libraryHandleSsl)
				return;
			SSL_CTX_free_ptr(ctx);
		}

		void SSL_CTX_set_verify(SSL_CTX *ctx, int mode, SSL_verify_cb callback)
		{
			if(!libraryHandleSsl)
				return;
			SSL_CTX_set_verify_ptr(ctx, mode, callback);
		}

		int SSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_CTX_use_certificate_file_ptr(ctx, file, type);
		}
		
		int SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_CTX_use_PrivateKey_file_ptr(ctx, file, type);
		}
		
		int SSL_CTX_check_private_key(const SSL_CTX *ctx)
		{
			if(!libraryHandleSsl)
				return -1;
			return SSL_CTX_check_private_key_ptr(ctx);
		}
	}

	ssl_context::ssl_context()
	{
		pContext = nullptr;
	}

	ssl_context::~ssl_context()
	{		
		destroy();
	}

	ssl_context::ssl_context(ssl_context &&other) noexcept
	{
		pContext = std::exchange(other.pContext, nullptr);
	}

	ssl_context &ssl_context::operator=(ssl_context &&other) noexcept
	{
		if(this != &other)
		{
			pContext = std::exchange(other.pContext, nullptr);
		}
		return *this;
	}

	bool ssl_context::create(const std::string &certificateFilePath, const std::string &privateKeyFilePath)
	{
		destroy();
		
		auto context = openssl::SSL_CTX_new(openssl::TLS_server_method());

		if(context != nullptr) 
		{
			if (openssl::SSL_CTX_use_certificate_file(context, certificateFilePath.c_str(), SSL_FILETYPE_PEM) <= 0) 
			{
				openssl::SSL_CTX_free(context);
				throw std::invalid_argument("SSL_CTX_use_certificate_file failed");
			}

			if (openssl::SSL_CTX_use_PrivateKey_file(context, privateKeyFilePath.c_str(), SSL_FILETYPE_PEM) <= 0) 
			{
				openssl::SSL_CTX_free(context);
				throw std::invalid_argument("SSL_CTX_use_PrivateKey_file failed");
			}

			if (!openssl::SSL_CTX_check_private_key(context)) 
			{
				openssl::SSL_CTX_free(context);
				throw std::invalid_argument("SSL_CTX_check_private_key failed");
			}

			pContext = reinterpret_cast<SSL_CTX*>(context);
			return true;
		}

		return false;
	}

	bool ssl_context::create()
	{
		destroy();
		pContext = (SSL_CTX*)openssl::SSL_CTX_new(openssl::TLS_method());
		return pContext != nullptr;
	}

	void ssl_context::destroy()
	{
		if(pContext)
		{
			openssl::SSL_CTX_free((SSL_CTX*)pContext);
			pContext = nullptr;
		}
		pContext = nullptr;
	}

	bool ssl_context::is_valid() const
	{
		return pContext != nullptr;
	}

	void ssl_context::set_validate_peer(bool validate)
	{
		if(!pContext)
			return;
		if(validate)
			openssl::SSL_CTX_set_verify((SSL_CTX*)pContext, SSL_VERIFY_PEER, nullptr);
		else
			openssl::SSL_CTX_set_verify((SSL_CTX*)pContext, SSL_VERIFY_NONE, nullptr);
	}

	ssl::ssl()
	{
		pSsl = nullptr;
	}

	ssl::~ssl()
	{
		destroy();
	}

	ssl::ssl(ssl &&other) noexcept
	{
		pSsl = std::exchange(other.pSsl, nullptr);
	}

	ssl &ssl::operator=(ssl &&other) noexcept
	{
		if(this != &other)
		{
			pSsl = std::exchange(other.pSsl, nullptr);
		}
		return *this;
	}

	bool ssl::create(ssl_context *context)
	{
		if(!context)
			return false;

		if(!context->pContext)
			return false;

		destroy();
		
		pSsl = openssl::SSL_new(context->pContext);
		return pSsl != nullptr;
	}

	void ssl::destroy()
	{
		if(pSsl)
		{
			openssl::SSL_shutdown(pSsl);
			openssl::SSL_free(pSsl);
		}
		pSsl = nullptr;
	}

	bool ssl::set_file_descriptor(int32_t fd)
	{
		if(!pSsl)
			return false;
		return openssl::SSL_set_fd(pSsl, fd) == 1;
	}

	bool ssl::set_hostname(const std::string &hostName)
	{
		if(!pSsl)
			return false;
		openssl::SSL_ctrl(pSsl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, (void*)hostName.c_str());
		return true;
	}


	bool ssl::connect()
	{
		if(!pSsl)
			return false;
		return openssl::SSL_connect(pSsl) == 1;
	}

	bool ssl::accept()
	{
		if(!pSsl)
			return false;
		return openssl::SSL_accept(pSsl) == 1;
	}

	int64_t ssl::read(void *buffer, size_t size)
	{
		if(!pSsl)
			return -1;
		return openssl::SSL_read(pSsl, buffer, size);
	}

	int64_t ssl::write(const void *buffer, size_t size)
	{
		if(!pSsl)
			return -1;
		return openssl::SSL_write(pSsl, buffer, size);
	}

	int64_t ssl::peek(void *buffer, size_t size)
	{
		if(!pSsl)
			return -1;
		return openssl::SSL_peek(pSsl, buffer, size);
	}

	bool ssl::read_all(void *buffer, size_t size)
	{
		uint8_t *ptr = static_cast<uint8_t*>(buffer);
        size_t totalRead = 0;

        while (totalRead < size) 
		{
            int64_t bytesRead = read(ptr + totalRead, size - totalRead);
            
            if (bytesRead < 0) 
			{
                // An error occurred
                return false;
            } 
			else if (bytesRead == 0) 
			{
                // Connection closed
                return false;
            }

            totalRead += bytesRead;
        }

        return true; // All bytes read successfully
	}

	bool ssl::write_all(const void *buffer, size_t size)
	{
		const uint8_t *ptr = static_cast<const uint8_t*>(buffer);
        size_t totalSent = 0;

        while (totalSent < size) 
		{
            int64_t bytesSent = write(ptr + totalSent, size - totalSent);
            
            if (bytesSent < 0) 
			{
                // An error occurred
                return false;
            } 
			else if (bytesSent == 0) 
			{
                // Connection closed
                return false;
            }

            totalSent += bytesSent;
        }

        return true; // All bytes sent successfully
	}

	bool ssl::is_valid() const
	{
		return pSsl != nullptr;
	}
}