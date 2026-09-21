#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
class HTTPRequest
{
public:
    HTTPRequest(const std::string& inbuffer)
    {
        std::istringstream line(inbuffer.substr(0, inbuffer.find("\r\n")));
        line >> method_ >> uri_ >> version_;
        size_t prev = inbuffer.find("\r\n") + 2;
        while (true)
        {
            size_t pos = inbuffer.find("\r\n", prev);
            if (prev == pos)
            {
                break;
            }
            std::string header_line = inbuffer.substr(prev, pos - prev);
            prev = pos + 2;
            size_t colon = header_line.find(": ");
            std::string key = header_line.substr(0, colon);
            std::string value = header_line.substr(colon + 2);
            headers_[key] = value;
        }
    }

private:
    std::string method_;
    std::string uri_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
};