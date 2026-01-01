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

#include "http_session_manager.hpp"
#include <random>
#include <algorithm>
#include <iostream>

namespace stw
{
	constexpr static size_t CLEANUP_THRESHOLD = 100;
	constexpr static uint32_t MAX_AGE_SECONDS = (60 * 60 * 24 * 30); // 30 days
	constexpr static const char *COOKIE_NAME = "SESSION_ID";
	
    void http_session_manager::start(http_request &request, http_response &response)
    {
		if ((requestCounter.fetch_add(1, std::memory_order_relaxed) + 1) % CLEANUP_THRESHOLD == 0)
        {
            cleanup();
			requestCounter.store(0);
        }

		std::string existingId;
		bool hasCookie = request.get_cookie(COOKIE_NAME, existingId);

		if(hasCookie && !existingId.empty())
		{
			std::shared_lock lock(sharedMutex);
			auto it = sessions.find(existingId);
			
			auto now = date_time::get_now();

			if (it != sessions.end() && it->second->expires > now)
			{
				// Session already exists, extend lifetime
				it->second->expires = now.add_seconds(MAX_AGE_SECONDS);
				return; 
			}
		}

		std::string sid = create_id();
		auto session = std::make_shared<http_session>();
		session->id = sid;
		session->expires = date_time::get_now().add_seconds(MAX_AGE_SECONDS);

		{
			std::unique_lock lock(sharedMutex);
			sessions[sid] = session;
		}

		http_cookie_options opts;
		opts.maxAge = MAX_AGE_SECONDS;
		opts.path = "/";
		opts.httpOnly = true;
		response.set_cookie(COOKIE_NAME, sid, &opts);
    }

    void http_session_manager::destroy(http_request& request, http_response& response)
    {
        std::string sid;

		if(!request.get_cookie(COOKIE_NAME, sid))
			return;

        std::unique_lock lock(sharedMutex);
        sessions.erase(sid);

		// Cookie is expired, remove it from the request
		auto it = request.cookies.find(COOKIE_NAME);

		if(it != request.cookies.end())
			request.cookies.erase(it);
        
        // Expire the cookie on the client side
		http_cookie_options options;
		options.maxAge = 0;    // Triggers immediate deletion
		options.path = "/";    // Path must match the original cookie path exactly
        response.set_cookie(COOKIE_NAME, "", &options);
    }

    bool http_session_manager::get_value(http_request& request, const std::string& key, std::string& value)
    {
        std::string sid;

		if(!request.get_cookie(COOKIE_NAME, sid))
		{
			return false;
		}

        std::shared_lock lock(sharedMutex); // Shared lock for reading
        auto it = sessions.find(sid);

		const auto now = date_time::get_now();
        
        if (it != sessions.end() && it->second->expires > now)
        {
            auto data_it = it->second->settings.find(key);
            if (data_it != it->second->settings.end())
            {
                value = data_it->second;
                return true;
            }
        }

        return false;
    }

    bool http_session_manager::set_value(http_request& request, const std::string& key, const std::string& value)
    {
        std::string sid;

		if(!request.get_cookie(COOKIE_NAME, sid))
		{
			return false;
		}

		std::cout << "SID " << sid << '\n';

        std::unique_lock lock(sharedMutex); // Unique lock for writing
        auto it = sessions.find(sid);

		const auto now = date_time::get_now();
        
        if (it != sessions.end() && it->second->expires > now)
        {
            it->second->settings[key] = value;
            return true;
        }

        return false;
    }

    std::string http_session_manager::create_id()
    {
		static thread_local std::random_device rd;
		static thread_local std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 15);
		
		const char* hex = "0123456789abcdef";
		std::string res;
		res.reserve(32);

		for (size_t i = 0; i < 32; ++i) 
		{
			res += hex[dis(gen)];
		}
		return res;
    }

    void http_session_manager::cleanup()
    {
		std::unique_lock lock(sharedMutex);
        const auto now = date_time::get_now();
        
        for (auto it = sessions.begin(); it != sessions.end(); )
        {
            if (it->second->expires < now)
                it = sessions.erase(it);
            else
                ++it;
        }
    }
}