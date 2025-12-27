#include "http_server.hpp"
#include "../system/signal.hpp"
#include "../system/string.hpp"
#include "../system/stringstream.hpp"
#include <memory>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <future>
#include <iostream>

namespace stw
{
//#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    http_server::http_server()
    {
        isRunning.store(false);

        threadPool = std::make_unique<stw::thread_pool>();

        stw::signal::register_handler([this](int32_t n)
                                      {
            if(n == SIGINT || n == SIGTERM)
            {
                isRunning.store(false);
            #if defined(_WIN32) || defined(__APPLE__)
                exit(0);
            #endif
            } });

        stw::signal::register_signal(SIGINT);
        stw::signal::register_signal(SIGTERM);
    #if !defined(_WIN32)
        stw::signal::register_signal(SIGPIPE);
    #endif
    }

    int http_server::run(const stw::http_config &config)
    {
        if (isRunning.load())
            return 1;

		this->config = config;

		if(!onRequest)
			throw std::runtime_error("onRequest callback is not set"); 

        if (!listener.bind(config.bindAddress, config.portHttp))
            return 2;

        if (!listener.listen(4096))
            return 3;

        listener.set_timeout(1);
        listener.set_no_delay(true);

        isRunning.store(true);

        const size_t threadCount = std::thread::hardware_concurrency();

        std::vector<std::unique_ptr<http_worker_context>> workers;

        for (size_t i = 0; i < threadCount; ++i)
        {
            workers.push_back(std::make_unique<http_worker_context>());
            workers.back()->thread = std::thread(&http_server::network_loop, this, workers.back().get());
        }

        std::cout << "Server started listening on http://" << config.bindAddress << ":" << config.portHttp << '\n';

        size_t nextWorker = 0;

        while (isRunning.load())
        {
			try
			{
				auto client = std::make_shared<stw::socket>();

				if (listener.accept(client.get()))
				{
					client->set_blocking(false);
					client->set_no_delay(true);

					if(!workers[nextWorker]->enqueue(client))
						client->close();
					nextWorker = (nextWorker + 1) % threadCount;
				}
			}
			catch(const std::bad_alloc &e)
			{
				std::cerr << "Failed to allocate memory for new client " << e.what() << "\n";
				// Wait a bit for other threads/workers to finish and free up memory
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
        }

        for (auto &worker : workers)
        {
            worker->stopFlag = true;
            worker->poller->notify();
			if(worker->thread.joinable())
            	worker->thread.join();
			
			std::shared_ptr<stw::socket> orphanedSocket;
			while (worker->queue.try_dequeue(orphanedSocket)) 
			{
				orphanedSocket.reset();
			}
        }

        return 0;
    }

    void http_server::network_loop(http_worker_context *worker)
    {
        std::vector<stw::poll_event_result> activeEvents;
        activeEvents.reserve(1024);

        while (!worker->stopFlag.load())
        {
			if(activeEvents.size() > 0)
				activeEvents.clear();

			int32_t eventCount = worker->poller->wait(activeEvents, 1000);

			auto now = std::chrono::steady_clock::now();

			if(now - worker->lastCleanup > std::chrono::seconds(5))
			{
				for (auto it = worker->contexts.begin(); it != worker->contexts.end();)
				{
					auto context = it->second;

					if(context->isLocked.load())
					{
						++it;
						continue;
					}

					if(std::chrono::duration_cast<std::chrono::seconds>(now - context->lastActivity).count() > worker->keepAliveTime)
					{
						int32_t fd = context->connection->get_file_descriptor();
						worker->poller->remove(fd);
						context->connection->close();
						it = worker->contexts.erase(it);
						continue;
					}
					else
					{
						++it;
					}
				}

				worker->lastCleanup = now;
			}

			std::shared_ptr<stw::socket> newConnection;

            while (worker->queue.try_dequeue(newConnection))
            {
                int32_t fd = newConnection->get_file_descriptor();
                worker->contexts[fd] = std::make_shared<http_context>(newConnection);
                worker->poller->add(fd, stw::poll_event_read);
            }

			if(eventCount == 0)
				continue;

            for (const auto &ev : activeEvents)
            {
                auto it = worker->contexts.find(ev.fd);
                
				if (it == worker->contexts.end())
                    continue;

                std::shared_ptr<http_context> context = it->second;

                if (ev.flags & stw::poll_event_error || ev.flags & stw::poll_event_disconnect)
                {
					if(context->isLocked.load())
						continue;

                    worker->remove(context, "disconnected by client");
                    continue;
                }

                if (ev.flags & stw::poll_event_read)
				{
					if(context->isLocked.load())
						continue;
                    on_read(worker, context->connection->get_file_descriptor());
				}

                if (ev.flags & stw::poll_event_write)
				{
					if(context->isLocked.load())
						continue;
                    on_write(worker, context->connection->get_file_descriptor());
				}
            }
        }

        worker->contexts.clear();
    }

    void http_server::on_read(http_worker_context *worker, int32_t fd)
    {
        auto it = worker->contexts.find(fd);
        
        if (it == worker->contexts.end()) 
            return;

        std::shared_ptr<http_context> context = it->second;

        char tempBuffer[8192];

        while (true) 
        {
            int64_t bytesRead = context->connection->read(tempBuffer, sizeof(tempBuffer));
            
            if (bytesRead > 0) 
            {
				if (context->requestBuffer.size() + bytesRead > config.maxHeaderSize) 
				{
					send_response(worker, context, 431);
					return;
				}

                context->requestBuffer.append(tempBuffer, bytesRead);
                size_t headerEnd = context->requestBuffer.find("\r\n\r\n");

                if (headerEnd != std::string::npos)
                {
                    if(!http_request::parse(context->requestBuffer, context->request))
					{
						send_response(worker, context, 400);
						return;
					}

					context->request.ip = context->connection->get_ip();

                    // Determine where the body starts
                    size_t headerTotalSize = headerEnd + 4;
                    size_t leftOverSize = context->requestBuffer.size() - headerTotalSize;

                    std::shared_ptr<network_stream> networkStream;
                    
                    if(leftOverSize > 0)
                    {
                        std::string initialContent = context->requestBuffer.substr(headerTotalSize);
                        networkStream = std::make_shared<network_stream>(context->connection, initialContent.data(), initialContent.size());
                    }
                    else
                    {
                        networkStream = std::make_shared<network_stream>(context->connection, nullptr, 0);
                    }

                    worker->poller->remove(context->connection->get_file_descriptor());

					context->isLocked.store(true);

					constexpr auto toKiloBytes = [](uint32_t bytes) constexpr {
						return bytes * 1024;
					};

					constexpr uint32_t MAX_CONTENT_SIZE = toKiloBytes(128);

					if(context->request.contentLength > MAX_CONTENT_SIZE)
					{
						if(threadPool->is_available())
						{
							threadPool->enqueue([this, worker, context, networkStream]() {
								process_request(worker, context, networkStream);
							});
						}
						else
						{
							send_response(worker, context, 503);
						}
					}
					else
					{
						process_request(worker, context, networkStream);
					}

                    return;
                }
            }
            else
            {
                if(bytesRead == -1)
                {
                    // Socket drained. Level-triggered epoll will wake us up later.
                    if (STW_SOCKET_ERR == STW_EAGAIN || STW_SOCKET_ERR == STW_EWOULDBLOCK) 
                        return;
					worker->remove(context, "failed to read more request header data from socket");
					return;
                }
                else
                {
					//Commented out because this disconnects connections that are keep-alive
					//Since we regularly check for 'zombie' connections they don't stick around for too long
                    //worker->remove(context.get(), "failed to read request header data from socket");
                    return;
                }
            }
        }
    }

	void http_server::process_request(http_worker_context *worker, std::shared_ptr<http_context> context, std::shared_ptr<network_stream> networkStream)
	{
		context->connection->set_blocking(true);

		try 
		{
			http_response response = onRequest(context->request, networkStream.get());
			context->response = std::move(response);
		} 
		catch (const std::exception& e) 
		{
			context->connection->set_blocking(false);
			send_response(worker, context, 500);
			return;
		}

		context->responseBuffer.reserve(1024);
		stw::stringstream responseStream(context->responseBuffer);

		responseStream << "HTTP/1.1 " << context->response.statusCode << "\r\n";

		// We do not convert the response header keys to lower case
		if(context->response.content)
			responseStream << "Content-Length: " << context->response.content->get_length() << "\r\n";
		else
			responseStream << "Content-Length: 0\r\n";

		bool keepAlive = true;
		bool mustClose = false;

		// Request header keys are converted to lower case because some clients don't follow standard
		if(context->request.headers.contains("connection"))
		{
			if(stw::string::compare(context->request.headers["connection"], "close", true))
				keepAlive = false;
		}
		else
		{
			if(context->request.httpVersion == "HTTP/1.0")
			{
				keepAlive = false;
				mustClose = true;
			}
		}

		// All fine and dandy, but response headers with a Connection header have precedence over the request Connection header
		if(context->response.headers.size() > 0)
		{
			if(context->response.headers.contains("Connection"))
			{
				//The exception is HTTP/1.0
				if(mustClose)
				{
					context->response.headers["Connection"] = "close";
					keepAlive = false;
				}
				else
				{
					context->response.headers["Connection"] = "keep-alive";
					keepAlive = true;
				}
			}
		}

		if(context->requestCount == worker->maxRequests)
			keepAlive = false;

		if(context->response.headers.size() > 0)
		{
			for(const auto& [key,value] : context->response.headers)
			{
				responseStream << key + ": " << value << "\r\n";
			}
		}
		else
		{
			if(keepAlive)
			{
				responseStream << "Connection: keep-alive\r\n";
				responseStream << "Keep-Alive: timeout=" << worker->keepAliveTime << ", max=" << worker->maxRequests << "\r\n";
			}
			else
			{
				responseStream << "Connection: close\r\n";
			}
		}

		responseStream << "\r\n";

		context->closeConnection = !keepAlive;

		context->connection->set_blocking(false);
		
		finalize_request(worker, context);
	}

    void http_server::finalize_request(http_worker_context* worker, std::shared_ptr<http_context> context) 
    {
        // Re-add the socket to the poller. 
        worker->poller->add(context->connection->get_file_descriptor(), stw::poll_event_write);
		context->isLocked.store(false);
        // Notify the poller so it recognizes the new FD registration immediately
        worker->poller->notify();
    }

	void http_server::send_response(http_worker_context *worker, std::shared_ptr<http_context> context, uint32_t statusCode)
	{
		worker->poller->remove(context->connection->get_file_descriptor());
		
		context->response.content = nullptr;
		context->responseBuffer = 	"HTTP/1.1 " + std::to_string(statusCode) + 
									"\r\nConnection: close\r\n\r\n";
		context->closeConnection = true;

		finalize_request(worker, context);
	}

    void http_server::on_write(http_worker_context *worker, int32_t fd)
    {
        auto it = worker->contexts.find(fd);
        
        if (it == worker->contexts.end()) 
            return;

        std::shared_ptr<http_context> context = it->second;

        if (!context->responseBuffer.empty())
        {
            while (context->headerBytesSent < context->responseBuffer.size()) 
            {
                const char* ptr = context->responseBuffer.data() + context->headerBytesSent;
                size_t remaining = context->responseBuffer.size() - context->headerBytesSent;

                int64_t sent = context->connection->write(ptr, remaining);

                if (sent > 0) 
                {
                    context->headerBytesSent += sent;
                } 
                else
                {
                    if(sent == -1)
                    {
                        if(STW_SOCKET_ERR == STW_EAGAIN || STW_SOCKET_ERR == STW_EWOULDBLOCK)
                            return;
                    }
                    else
                    {
                        worker->remove(context, "failed to write response header to socket");
                        return;
                    }
                } 
            }
        }

        if (context->response.content) 
        {
            char tempBuffer[8192];
            
            while (true) 
            {
                uint64_t readOffset = context->response.content->get_read_offset();

                int64_t bytesRead = context->response.content->read(tempBuffer, sizeof(tempBuffer));
                if (bytesRead <= 0) 
                    goto request_finished; // EOF

                int64_t bytesSent = context->connection->write(tempBuffer, bytesRead);

                if (bytesSent <= 0) 
                {
                    if(bytesSent == -1)
                    {
                        if (STW_SOCKET_ERR == STW_EAGAIN || STW_SOCKET_ERR == STW_EWOULDBLOCK) 
                        {
                            // We sent 0, but read 'bytesRead'. Rewind to start of this chunk.
                            context->response.content->seek(readOffset, stw::seek_origin_begin);
                            return; 
                        }
                    } 
                    worker->remove(context, "failed to write response content to socket");
                    return;
                }
                
                if (bytesSent < bytesRead) 
                {
                    // PARTIAL WRITE: Rewind to the exact byte where the socket stopped
                    uint64_t resumePosition = readOffset + bytesSent;
                    context->response.content->seek(resumePosition, stw::seek_origin_begin);
                    return;
                }
            }
        }
    request_finished:
		context->requestCount++;

		if(context->requestCount >= worker->maxRequests)
			context->closeConnection = true;

		if (context->closeConnection) 
        {
            worker->remove(context, "request finished (Connection: close)");
            return;
        }

		context->requestBuffer.clear();
        context->responseBuffer.clear();
        context->headerBytesSent = 0;
        context->response.content.reset();
		context->lastActivity = std::chrono::steady_clock::now();
		context->request.headers.clear();
		context->response.headers.clear();
        
        worker->poller->modify(context->connection->get_file_descriptor(), stw::poll_event_read);
    }

    http_worker_context::http_worker_context()
    {
        stopFlag.store(false);
        poller = stw::poller::create();
		lastCleanup = std::chrono::steady_clock::now();
		maxRequests = 100;
		keepAliveTime = 15;
    }

	bool http_worker_context::enqueue(std::shared_ptr<stw::socket> s)
    {
        if(queue.enqueue(s))
        {
            poller->notify();
			return true;
        }
        return false;
    }

    void http_worker_context::remove(std::shared_ptr<http_context> context, const char *sender)
    {
        if(!context.get())
            return;

        int32_t fd = context->connection->get_file_descriptor();
        poller->remove(fd);
        context->connection->close();
		context->connection.reset();
		context->response.content.reset();
        contexts.erase(fd);
    }
//#endif
}