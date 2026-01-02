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

#ifndef STW_HTTP_SESSION_MANAGER_HPP
#define STW_HTTP_SESSION_MANAGER_HPP

#include "http.hpp"
#include "../system/date_time.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <atomic>
#include <shared_mutex>

namespace stw
{
	struct http_session
	{
		std::string id;
		std::unordered_map<std::string,std::string> settings;
		date_time expires;
	};

	class http_session_manager
	{
	public:
		http_session_manager() : requestCounter(0) {}
		http_session_manager(const http_session_manager&) = delete;
		void operator=(const http_session_manager&) = delete;
		void start(http_request &request, http_response &response);
		void destroy(http_request &request, http_response &response);
		bool get_id(http_request &request, std::string &sessionId);
		bool get_value(http_request &request, const std::string &key, std::string& value);
		bool set_value(http_request &request, const std::string &key, const std::string& value);
		bool key_exists(http_request &request, const std::string &key);
		void invalidate_sessions_with_key_and_value(const std::string &key, const std::string &value);
		static http_session_manager *get_instance()
		{
			static http_session_manager instance;
			return &instance;
		}
	private:
		std::unordered_map<std::string,std::shared_ptr<http_session>> sessions;
		std::shared_mutex sharedMutex;
		std::atomic<uint32_t> requestCounter;
		std::string create_id();
		void cleanup();		
	};
}

#endif