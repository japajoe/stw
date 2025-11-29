#ifndef STW_HTTP_ROUTER_HPP
#define STW_HTTP_ROUTER_HPP

#include "stw_http.hpp"
#include "stw_http_server.hpp"
#include <vector>
#include <regex>
#include <functional>

namespace stw
{
	using http_request_handler = std::function<void(stw::http_connection *connection, const stw::http_request_info &request)>;

	struct http_route
	{
		std::regex regex;
		http_method method;
		http_request_handler handler;
	};
	
	class http_router
	{
	public:
		void add(http_method method, const std::string &route, http_request_handler handler);
		http_route *get(const std::string &route);
	private:
		std::vector<http_route> routes;
	};
}

#endif