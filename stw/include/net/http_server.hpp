#ifndef STW_HTTP_SERVER_HPP
#define STW_HTTP_SERVER_HPP

#include "socket.hpp"
#include "poller.hpp"
#include "http.hpp"
#include "network_stream.hpp"
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
#include <chrono>
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
		time_point lastActivity;
		std::atomic<bool> isLocked;
        http_context()
        {
            connection = nullptr;
            headerBytesSent = 0;
			closeConnection = false;
			requestCount = 0;
			lastActivity = std::chrono::steady_clock::now();
			isLocked.store(false);
        }
		http_context(std::shared_ptr<stw::socket> s)
        {
			connection = s;
            headerBytesSent = 0;
			closeConnection = false;
			requestCount = 0;
			lastActivity = std::chrono::steady_clock::now();
			isLocked.store(false);
        }
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
		time_point lastCleanup;
		uint32_t maxRequests;
		uint32_t keepAliveTime;
        std::atomic<bool> stopFlag;
    };

    using request_handler = std::function<http_response(const http_request &request, network_stream *stream)>;

    class http_server
    {
    public:
        request_handler onRequest;
        http_server();
        int run(const std::string &bindAddress, uint16_t port, uint32_t backlog);
    private:
        stw::socket listener;
        std::atomic<bool> isRunning;
        std::unique_ptr<stw::thread_pool> threadPool;
        void network_loop(http_worker_context *worker);
        void on_read(http_worker_context *worker, int32_t fd);
        void on_write(http_worker_context *worker, int32_t fd);
		void process_request(http_worker_context *worker, std::shared_ptr<http_context> context, std::shared_ptr<network_stream> networkStream);
        void finalize_request(http_worker_context *worker, std::shared_ptr<http_context> context);
		void send_response(http_worker_context *worker, std::shared_ptr<http_context> context, uint32_t statusCode);
    };
}

#endif