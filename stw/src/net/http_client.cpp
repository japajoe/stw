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

#include "http_client.hpp"
#include "../core/platform.hpp"
#include "../system/runtime.hpp"
#include "../system/string.hpp"
#include <vector>
#include <sstream>
#include <cstring>
#include <iostream>
#include <algorithm>

namespace stw
{
    #define CURL_GLOBAL_SSL (1<<0)
    #define CURL_GLOBAL_WIN32 (1<<1)
    #define CURL_GLOBAL_ALL (CURL_GLOBAL_SSL|CURL_GLOBAL_WIN32)
    #define CURL_GLOBAL_DEFAULT CURL_GLOBAL_ALL

    typedef size_t (*curl_setopt_callback)(void* contents, size_t size, size_t nmemb, void *userp);

    typedef void CURL;

    typedef enum {

    } CURLINFO;

    typedef enum {
        CURLE_OK = 0,
        CURLE_UNSUPPORTED_PROTOCOL,    
        CURLE_FAILED_INIT,             
        CURLE_URL_MALFORMAT,           
        CURLE_NOT_BUILT_IN,            
        CURLE_COULDNT_RESOLVE_PROXY,   
        CURLE_COULDNT_RESOLVE_HOST,    
        CURLE_COULDNT_CONNECT,         
        CURLE_WEIRD_SERVER_REPLY,      
        CURLE_REMOTE_ACCESS_DENIED,    
        CURLE_FTP_ACCEPT_FAILED,       
        CURLE_FTP_WEIRD_PASS_REPLY,    
        CURLE_FTP_ACCEPT_TIMEOUT,      
        CURLE_FTP_WEIRD_PASV_REPLY,    
        CURLE_FTP_WEIRD_227_FORMAT,    
        CURLE_FTP_CANT_GET_HOST,       
        CURLE_HTTP2,                   
        CURLE_FTP_COULDNT_SET_TYPE,    
        CURLE_PARTIAL_FILE,            
        CURLE_FTP_COULDNT_RETR_FILE,   
        CURLE_OBSOLETE20,              
        CURLE_QUOTE_ERROR,             
        CURLE_HTTP_RETURNED_ERROR,     
        CURLE_WRITE_ERROR,             
        CURLE_OBSOLETE24,              
        CURLE_UPLOAD_FAILED,           
        CURLE_READ_ERROR,              
        CURLE_OUT_OF_MEMORY,           
        CURLE_OPERATION_TIMEDOUT,      
        CURLE_OBSOLETE29,              
        CURLE_FTP_PORT_FAILED,         
        CURLE_FTP_COULDNT_USE_REST,    
        CURLE_OBSOLETE32,              
        CURLE_RANGE_ERROR,             
        CURLE_HTTP_POST_ERROR,         
        CURLE_SSL_CONNECT_ERROR,       
        CURLE_BAD_DOWNLOAD_RESUME,     
        CURLE_FILE_COULDNT_READ_FILE,  
        CURLE_LDAP_CANNOT_BIND,        
        CURLE_LDAP_SEARCH_FAILED,      
        CURLE_OBSOLETE40,              
        CURLE_FUNCTION_NOT_FOUND,      
        CURLE_ABORTED_BY_CALLBACK,     
        CURLE_BAD_FUNCTION_ARGUMENT,   
        CURLE_OBSOLETE44,              
        CURLE_INTERFACE_FAILED,        
        CURLE_OBSOLETE46,              
        CURLE_TOO_MANY_REDIRECTS,      
        CURLE_UNKNOWN_OPTION,          
        CURLE_TELNET_OPTION_SYNTAX,    
        CURLE_OBSOLETE50,              
        CURLE_OBSOLETE51,              
        CURLE_GOT_NOTHING,             
        CURLE_SSL_ENGINE_NOTFOUND,     
        CURLE_SSL_ENGINE_SETFAILED,    
        CURLE_SEND_ERROR,              
        CURLE_RECV_ERROR,              
        CURLE_OBSOLETE57,              
        CURLE_SSL_CERTPROBLEM,         
        CURLE_SSL_CIPHER,              
        CURLE_PEER_FAILED_VERIFICATION, 
        CURLE_BAD_CONTENT_ENCODING,    
        CURLE_LDAP_INVALID_URL,        
        CURLE_FILESIZE_EXCEEDED,       
        CURLE_USE_SSL_FAILED,          
        CURLE_SEND_FAIL_REWIND,        
        CURLE_SSL_ENGINE_INITFAILED,   
        CURLE_LOGIN_DENIED,            
        CURLE_TFTP_NOTFOUND,           
        CURLE_TFTP_PERM,               
        CURLE_REMOTE_DISK_FULL,        
        CURLE_TFTP_ILLEGAL,            
        CURLE_TFTP_UNKNOWNID,          
        CURLE_REMOTE_FILE_EXISTS,      
        CURLE_TFTP_NOSUCHUSER,         
        CURLE_CONV_FAILED,             
        CURLE_CONV_REQD,               
        CURLE_SSL_CACERT_BADFILE,      
        CURLE_REMOTE_FILE_NOT_FOUND,   
        CURLE_SSH,                     
        CURLE_SSL_SHUTDOWN_FAILED,     
        CURLE_AGAIN,                   
        CURLE_SSL_CRL_BADFILE,         
        CURLE_SSL_ISSUER_ERROR,        
        CURLE_FTP_PRET_FAILED,         
        CURLE_RTSP_CSEQ_ERROR,         
        CURLE_RTSP_SESSION_ERROR,      
        CURLE_FTP_BAD_FILE_LIST,       
        CURLE_CHUNK_FAILED,            
        CURLE_NO_CONNECTION_AVAILABLE, 
        CURLE_SSL_PINNEDPUBKEYNOTMATCH, 
        CURLE_SSL_INVALIDCERTSTATUS,   
        CURLE_HTTP2_STREAM,            
        CURLE_RECURSIVE_API_CALL,      
        CURLE_AUTH_ERROR,              
        CURLE_HTTP3,                   
        CURLE_QUIC_CONNECT_ERROR,      
        CURLE_PROXY,                   
        CURL_LAST 
    } CURLcode;
    
    struct curl_slist {
        char *data;
        struct curl_slist *next;
    };

    typedef enum {
        CURLOPT_URL = 10002,
        CURLOPT_WRITEFUNCTION = 20011,
        CURLOPT_WRITEDATA = 10001,
        CURLOPT_HEADERFUNCTION = 20079,
        CURLOPT_HEADERDATA = 10029,
        CURLOPT_SSL_VERIFYPEER = 64,
        CURLOPT_SSL_VERIFYHOST = 81,
        CURLOPT_HTTPHEADER = 10023,
        CURLOPT_POSTFIELDS = 10015,
        CURLOPT_POSTFIELDSIZE = 60,
    } CURLoption;

	namespace curl
	{
		typedef CURLcode (*curl_global_init_t)(long flags);
		typedef void (*curl_global_cleanup_t)(void);
		typedef CURL *(*curl_easy_init_t)(void);
		typedef void (*curl_easy_cleanup_t)(CURL*);
		typedef CURLcode (*curl_easy_setopt_t)(CURL*, CURLoption, ...);
		typedef CURLcode (*curl_easy_perform_t)(CURL*);
		typedef CURLcode (*curl_easy_getinfo_t)(CURL*, CURLINFO, ...);
		typedef void (*curl_slist_free_all_t)(curl_slist*);
		typedef curl_slist *(*curl_slist_append_t)(curl_slist*, const char*);
		typedef const char *(*curl_easy_strerror_t)(CURLcode);

		curl_global_init_t curl_global_init_ptr = nullptr;
		curl_global_cleanup_t curl_global_cleanup_ptr = nullptr;
		curl_easy_init_t curl_easy_init_ptr = nullptr;
		curl_easy_cleanup_t curl_easy_cleanup_ptr = nullptr;
		curl_easy_setopt_t curl_easy_setopt_ptr = nullptr;
		curl_easy_perform_t curl_easy_perform_ptr = nullptr;
		curl_easy_getinfo_t curl_easy_getinfo_ptr = nullptr;
		curl_slist_free_all_t curl_slist_free_all_ptr = nullptr;
		curl_slist_append_t curl_slist_append_ptr = nullptr;
		curl_easy_strerror_t curl_easy_strerror_ptr = nullptr;

		static void *libraryHandle = nullptr;

		bool is_initialized(void *fn, const char *name)
		{
			if(fn)
				return true;
			
			std::cerr << "Failed to loaded function: " << name << '\n';

			if(libraryHandle)
				stw::runtime::unload_library(libraryHandle);
			
            libraryHandle = nullptr;
			
            return false;
		}

		bool load_library(const std::string &libraryPath)
		{
			libraryHandle = stw::runtime::load_library(libraryPath);

			if(!libraryHandle)
				return false;
			
			curl_global_init_ptr = (curl_global_init_t)stw::runtime::get_symbol(libraryHandle, "curl_global_init");
			curl_global_cleanup_ptr = (curl_global_cleanup_t)stw::runtime::get_symbol(libraryHandle, "curl_global_cleanup");
			curl_easy_init_ptr = (curl_easy_init_t)stw::runtime::get_symbol(libraryHandle, "curl_easy_init");
			curl_easy_cleanup_ptr = (curl_easy_cleanup_t)stw::runtime::get_symbol(libraryHandle, "curl_easy_cleanup");
			curl_easy_setopt_ptr = (curl_easy_setopt_t)stw::runtime::get_symbol(libraryHandle, "curl_easy_setopt");
			curl_easy_perform_ptr = (curl_easy_perform_t)stw::runtime::get_symbol(libraryHandle, "curl_easy_perform");
			curl_easy_getinfo_ptr = (curl_easy_getinfo_t)stw::runtime::get_symbol(libraryHandle, "curl_easy_getinfo");
			curl_slist_free_all_ptr = (curl_slist_free_all_t)stw::runtime::get_symbol(libraryHandle, "curl_slist_free_all");
			curl_slist_append_ptr = (curl_slist_append_t)stw::runtime::get_symbol(libraryHandle, "curl_slist_append");
			curl_easy_strerror_ptr = (curl_easy_strerror_t)stw::runtime::get_symbol(libraryHandle, "curl_easy_strerror");

			if(!is_initialized((void*)curl_global_init_ptr, "curl_global_init_ptr"))
				return false;
			if(!is_initialized((void*)curl_global_cleanup_ptr, "curl_global_cleanup_ptr"))
				return false;
			if(!is_initialized((void*)curl_easy_init_ptr, "curl_easy_init_ptr"))
				return false;
			if(!is_initialized((void*)curl_easy_cleanup_ptr, "curl_easy_cleanup_ptr"))
				return false;
			if(!is_initialized((void*)curl_easy_setopt_ptr, "curl_easy_setopt_ptr"))
				return false;
			if(!is_initialized((void*)curl_easy_perform_ptr, "curl_easy_perform_ptr"))
				return false;
			if(!is_initialized((void*)curl_easy_getinfo_ptr, "curl_easy_getinfo_ptr"))
				return false;
			if(!is_initialized((void*)curl_slist_free_all_ptr, "curl_slist_free_all_ptr"))
				return false;
			if(!is_initialized((void*)curl_slist_append_ptr, "curl_slist_append_ptr"))
				return false;
			if(!is_initialized((void*)curl_easy_strerror_ptr, "curl_easy_strerror_ptr"))
				return false;

			return true;
		}

		bool is_loaded()
		{
			return libraryHandle != nullptr;
		}

		CURLcode global_init(long flags)
		{
			if(!libraryHandle)
				return CURLE_FAILED_INIT;
			return curl_global_init_ptr(flags);
		}

		void global_cleanup()
		{
			if(!libraryHandle)
				return;
			curl_global_cleanup_ptr();
		}

		CURL *easy_init()
		{
			if(!libraryHandle)
				return nullptr;
			return curl_easy_init_ptr();
		}

		void easy_cleanup(CURL *c)
		{
			if(!libraryHandle)
				return;
			curl_easy_cleanup_ptr(c);
		}

		CURLcode easy_setopt(CURL *c, CURLoption opt, const char *data)
		{
			if(!libraryHandle)
				return CURLE_FAILED_INIT;
			return curl_easy_setopt_ptr(c, opt, data);
		}

		CURLcode easy_setopt(CURL *c, CURLoption opt, curl_setopt_callback data)
		{
			if(!libraryHandle)
				return CURLE_FAILED_INIT;
			return curl_easy_setopt_ptr(c, opt, data);
		}

		CURLcode easy_setopt(CURL *c, CURLoption opt, void *data)
		{
			if(!libraryHandle)
				return CURLE_FAILED_INIT;
			return curl_easy_setopt_ptr(c, opt, data);
		}

		CURLcode easy_setopt(CURL *c, CURLoption opt, size_t data)
		{
			if(!libraryHandle)
				return CURLE_FAILED_INIT;
			return curl_easy_setopt_ptr(c, opt, data);
		}

		CURLcode easy_setopt(CURL *c, CURLoption opt, long data)
		{
			if(!libraryHandle)
				return CURLE_FAILED_INIT;
			return curl_easy_setopt_ptr(c, opt, data);
		}

		CURLcode easy_perform(CURL *c)
		{
			if(!libraryHandle)
				return CURLE_FAILED_INIT;
			return curl_easy_perform_ptr(c);
		}

		CURLcode easy_getinfo(CURL *c, CURLINFO info, void *data)
		{
			if(!libraryHandle)
				return CURLE_FAILED_INIT;
			return curl_easy_getinfo_ptr(c, info, data);
		}

		void slist_free_all(curl_slist *sl)
		{
			if(!libraryHandle)
				return;
			curl_slist_free_all_ptr(sl);
		}

		curl_slist *slist_append(curl_slist *sl, const char * str)
		{
			if(!libraryHandle)
				return nullptr;
			return curl_slist_append_ptr(sl, str);
		}

		const char *easy_strerror(CURLcode code)
		{
			return curl_easy_strerror_ptr(code);
		}

		bool load_library()
		{
			if(is_loaded())
				return true;
		
			std::string curlPath;
			
		#if defined(STW_PLATFORM_WINDOWS)
			curlPath = "libcurl-x64.dll";
		#elif defined(STW_PLATFORM_LINUX)
			stw::runtime::find_library_path("libcurl.so", curlPath);
		#elif defined(STW_PLATFORM_MAC)
			//Not implemented yet
			return false;
		#endif
		
			if(curlPath.size() > 0)
			{
				if(load_library(curlPath))
				{
					global_init(CURL_GLOBAL_DEFAULT);
					return true;
				}
			}

			return false;
		}

		void close_library()
		{
			if(libraryHandle)
				stw::runtime::unload_library(libraryHandle);
			libraryHandle = nullptr;
		}
	}

    static void write_error(const std::string &message) 
	{
		std::cerr << message << '\n';
    }

	http_client::http_client()
	{
        validateCertificate = true;
	}

    void http_client::set_validate_certificate(bool validate)
    {
        validateCertificate = validate;
    }

    bool http_client::validate_certificate() const
    {
        return validateCertificate;
    }

    bool http_client::get(const http_request &req, http_response &res)
    {
        if(!curl::is_loaded())
            return false;

        CURL *gCurl = curl::easy_init();
        if (!gCurl) 
            return false;

        std::string requestHeader;

        curl::easy_setopt(gCurl, CURLOPT_URL, req.get_url().c_str());
        curl::easy_setopt(gCurl, CURLOPT_WRITEFUNCTION, write_callback);
        curl::easy_setopt(gCurl, CURLOPT_WRITEDATA, this);
        curl::easy_setopt(gCurl, CURLOPT_HEADERFUNCTION, header_callback);
        curl::easy_setopt(gCurl, CURLOPT_HEADERDATA, &requestHeader);

        // Disable SSL peer verification
        if(!validateCertificate)
        {
            curl::easy_setopt(gCurl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl::easy_setopt(gCurl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

		struct curl_slist* requestHeaderList = nullptr;

        auto requestHeaders = req.get_headers();

		if(requestHeaders.size() > 0)
		{
			for(const auto &h : requestHeaders)
			{
				std::string s = h.first + ": " + h.second;
				requestHeaderList = curl::slist_append(requestHeaderList, s.c_str());
			}
			
			curl::easy_setopt(gCurl, CURLOPT_HTTPHEADER, requestHeaderList);
		}

        CURLcode result = curl::easy_perform(gCurl);

        if (result != CURLE_OK) 
        {
			std::string error = "GET request failed: " + std::string(curl::easy_strerror(result));
            write_error(error);
			if(requestHeaderList)
				curl::slist_free_all(requestHeaderList);
            curl::easy_cleanup(gCurl);
            return false;
        }

		if(requestHeaderList)
			curl::slist_free_all(requestHeaderList);

        if(!parse_header(requestHeader, res.header, res.statusCode, res.contentLength))
        {
            write_error("Failed to parse header");
            curl::easy_cleanup(gCurl);
            return false;
        }

        curl::easy_cleanup(gCurl);

        return true;
    }

    bool http_client::post(const http_request &req, http_response &res)
    {
        if(!curl::is_loaded())
            return false;

        CURL *gCurl = curl::easy_init();
        
        if (!gCurl) 
            return false;

        curl::easy_setopt(gCurl, CURLOPT_URL, req.get_url().c_str());

        uint8_t *pContent = req.get_content();
        size_t contentLength = req.get_content_length();

        if(pContent && contentLength > 0)
        {
            curl::easy_setopt(gCurl, CURLOPT_POSTFIELDS, pContent);
            curl::easy_setopt(gCurl, CURLOPT_POSTFIELDSIZE, contentLength);
        }
        
        curl::easy_setopt(gCurl, CURLOPT_WRITEFUNCTION, write_callback);
        curl::easy_setopt(gCurl, CURLOPT_WRITEDATA, this);
        curl::easy_setopt(gCurl, CURLOPT_HEADERFUNCTION, header_callback);

        // Disable SSL peer verification
        if(!validateCertificate)
        {
            curl::easy_setopt(gCurl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl::easy_setopt(gCurl, CURLOPT_SSL_VERIFYHOST, 0L);
        }
        
        std::string responseHeader;
        curl::easy_setopt(gCurl, CURLOPT_HEADERDATA, &responseHeader);

        struct curl_slist* requestHeaderList = nullptr;

        if(pContent && contentLength > 0)
        {
            std::string s = "Content-Type: " + req.get_content_type();
            requestHeaderList = curl::slist_append(requestHeaderList, s.c_str());
        }

        auto requestHeaders = req.get_headers();

		if(requestHeaders.size())
		{
			for(const auto &h : requestHeaders)
			{
				std::string s = h.first + ": " + h.second;
				requestHeaderList = curl::slist_append(requestHeaderList, s.c_str());
			}
		}

        curl::easy_setopt(gCurl, CURLOPT_HTTPHEADER, requestHeaderList);

        CURLcode result = curl::easy_perform(gCurl);

        if (result != CURLE_OK) 
        {
			std::string error = "POST request failed: " + std::string(curl::easy_strerror(result));
			write_error(error);
            curl::slist_free_all(requestHeaderList);
            curl::easy_cleanup(gCurl);
            return false;
        }

        curl::slist_free_all(requestHeaderList);

        if(!parse_header(responseHeader, res.header, res.statusCode, res.contentLength))
        {
            curl::easy_cleanup(gCurl);
            return false;
        }

        curl::easy_cleanup(gCurl);

        return true;
    }

    bool http_client::parse_header(const std::string &responseText, http_headers &header, int &statusCode, uint64_t &contentLength)
    {
        std::istringstream responseStream(responseText);
        std::string line;
        size_t count = 0;

        while(std::getline(responseStream, line))
        {
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

            if(line.size() == 0)
                continue;

            if(count == 0)
            {
                if(line[line.size() - 1] == ' ')
                    line.pop_back();

                auto parts = string::split(line, ' ', 0);
                
                if(parts.size() < 2)
                    return false;

                if(!string::try_parse_int32(parts[1], statusCode))
                    return false;
            }
            else
            {
                auto parts = string::split(line, ':', 2);

                if(parts.size() == 2)
                {
                    parts[0] = string::to_lower(parts[0]);
                    parts[1] = string::trim_start(parts[1]);
                    header[parts[0]] = parts[1];
                }
            }

            count++;
        }

        if(header.contains("content-length"))
        {
            if(!string::try_parse_uint64(header["content-length"], contentLength))
                contentLength = 0;
        }

        return count > 0;
    }

    size_t http_client::write_callback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        http_client *pClient = reinterpret_cast<http_client*>(userp);
        size_t totalSize = size * nmemb;
        if(pClient->onResponse)
            pClient->onResponse(contents, totalSize);
        return totalSize;
    }
    
    size_t http_client::header_callback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        size_t totalSize = size * nmemb;
        std::string* str = static_cast<std::string*>(userp);
        str->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
}