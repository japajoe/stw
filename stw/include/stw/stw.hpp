#ifndef STW_STW_HPP
#define STW_STW_HPP

#include "core/platform.hpp"
#include "net/http.hpp"
#include "net/http_server.hpp"
#include "net/http_controller.hpp"
#include "net/http_router.hpp"
#include "net/http_client.hpp"
#include "net/http_session_manager.hpp"
#include "net/http_stream.hpp"
#include "net/http_config.hpp"
#include "net/poller.hpp"
#include "net/ssl.hpp"
#include "net/socket.hpp"
#include "net/web_socket.hpp"
#include "system/arg_parser.hpp"
#include "system/crypto.hpp"
#include "system/queue.hpp"
#include "system/ini_reader.hpp"
#include "system/runtime.hpp"
#include "system/string.hpp"
#include "system/file_cache.hpp"
#include "system/file.hpp"
#include "system/date_time.hpp"
#include "system/thread_pool.hpp"
#include "system/stream.hpp"
#include "system/signal.hpp"
#include "system/directory.hpp"
#include "templating/templ.hpp"

namespace stw
{
	// Loads dynamic libraries such as curl/openssl/zlib. Call this method before using the library
	void load_library();
}

#endif