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

#ifndef STW_HTTP_SERVER_HPP
#define STW_HTTP_SERVER_HPP

#include "socket.hpp"
#include "poller.hpp"
#include "http.hpp"
#include "http_config.hpp"
#include "http_stream.hpp"
#include "../system/thread_pool.hpp"
#include "../system/queue.hpp"
#include "../system/stream.hpp"
#include "../system/date_time.hpp"
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <functional>
#include <unordered_map>

namespace stw
{
    struct http_context
    {
        std::shared_ptr<stw::socket> connection;
        std::string requestBuffer;
		std::string responseBuffer;
        http_request request;
        http_response response;
        uint64_t headerBytesSent;
		uint32_t requestCount;
		bool closeConnection;
		stw::date_time lastActivity;
		std::atomic<bool> isLocked;
        http_context();
		http_context(std::shared_ptr<stw::socket> s);
    };

    struct http_worker_context
    {
        http_worker_context();
		bool enqueue(std::shared_ptr<stw::socket> s);
        void remove(std::shared_ptr<http_context> context, const char *sender);
        std::thread thread;
		stw::queue<std::shared_ptr<stw::socket>,1024> queue;
        std::unordered_map<int32_t,std::shared_ptr<http_context>> contexts;
        std::unique_ptr<stw::poller> poller;
		stw::date_time lastCleanup;
		uint32_t maxRequests;
		uint32_t keepAliveTime;
        std::atomic<bool> stopFlag;
    };

    using request_handler = std::function<http_response(const http_request &request, http_stream *stream)>;

    class http_server
    {
    public:
        request_handler onRequest;
        http_server();
        int run(const stw::http_config &config);
    private:
        stw::socket listener;
		stw::http_config config;
        std::atomic<bool> isRunning;
        std::unique_ptr<stw::thread_pool> threadPool;
        void worker_update(http_worker_context *worker);
        void on_read(http_worker_context *worker, int32_t fd);
        void on_write(http_worker_context *worker, int32_t fd);
		void process_request(http_worker_context *worker, std::shared_ptr<http_context> context, std::shared_ptr<http_stream> networkStream);
        void finalize_request(http_worker_context *worker, std::shared_ptr<http_context> context);
		void send_response(http_worker_context *worker, std::shared_ptr<http_context> context, uint32_t statusCode);
    };
}

#endif