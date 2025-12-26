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

#include "http.hpp"
#include "../system/string.hpp"
#include <map>
#include <filesystem>
#include <algorithm>

namespace stw
{
    std::string http_method_to_string(http_method m)
    {
        switch(m)
        {
        case http_method_get:
            return "GET";
        case http_method_post:
            return "POST";
        case http_method_put:
            return "PUT";
        case http_method_patch:
            return "PATCH";
        case http_method_delete:
            return "DELETE";
        case http_method_head:
            return "HEAD";
        case http_method_options:
            return "OPTIONS";
        case http_method_trace:
            return "TRACE";
        case http_method_connect:
            return "CONNECT";
        default:
            return "UNKNOWN";
        }
    }

    http_method string_to_http_method(const std::string &str)
    {
        std::string method = stw::string::to_upper(str);
        if(method == "GET")
            return http_method_get;
        else if(method == "POST")
            return http_method_post;
        else if(method == "PUT")
            return http_method_put;
        else if(method == "PATCH")
            return http_method_patch;
        else if(method == "DELETE")
            return http_method_delete;
        else if(method == "HEAD")
            return http_method_head;
        else if(method == "OPTIONS")
            return http_method_options;
        else if(method == "TRACE")
            return http_method_trace;
        else if(method == "CONNECT")
            return http_method_connect;
        else
            return http_method_unknown;
    }

    static std::map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".bmp", "image/bmp"},
        {".svg", "image/svg+xml"},
        {".pdf", "application/pdf"},
        {".txt", "text/plain"},
        {".xml", "application/xml"}
    };

    std::string get_http_content_type(const std::string &filePath)
    {
        std::filesystem::path path(filePath);
        std::string extension = path.extension().string();

        auto it = mimeTypes.find(extension);
        if (it != mimeTypes.end())
            return it->second;
        
        return "application/octet-stream";
    }
}