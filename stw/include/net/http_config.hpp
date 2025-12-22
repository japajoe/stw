#ifndef STW_HTTP_CONFIG_HPP
#define STW_HTTP_CONFIG_HPP

#include <string>
#include <cstdint>

namespace stw
{
    struct http_config
	{
        uint16_t portHttp;
        uint16_t portHttps;
        uint32_t maxHeaderSize;
        std::string bindAddress;
        std::string certificatePath;
        std::string privateKeyPath;
		std::string publicHtmlPath;
		std::string privateHtmlPath;
        std::string hostName;
        bool useHttpsForwarding;
        void load_default();
		bool load_from_file(const std::string &filePath);
    };
}

#endif