#include "stw_http_router.hpp"

namespace stw
{
	void http_router::add(http_method method, const std::string &route, http_request_handler handler)
	{
		http_route r = {
			.regex = std::regex(route),
			.method = method,
			.handler = handler
		};

		routes.push_back(r);
	}

	http_route *http_router::get(const std::string &url)
	{
		for (auto &route : routes) 
        {
            std::smatch match;

            if (std::regex_match(url, match, route.regex))
                return &route;
        }

        return nullptr;
	}
}