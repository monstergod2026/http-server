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
                prev = prev + 2;
                break;
            }
            std::string header_line = inbuffer.substr(prev, pos - prev);
            prev = pos + 2;
            size_t colon = header_line.find(": ");
            std::string key = header_line.substr(0, colon);
            std::string value = header_line.substr(colon + 2);
            headers_[key] = value;
        }
        if (method_ == "POST")
        {
            auto it = headers_.find("Content-Length");
            if (it != headers_.end())
            {
                int n = std::stoi(it->second);
                body_ = inbuffer.substr(prev, n);
            }
            else
            {
                // todo 返回错误码
            }
        }
    }
    std::string Getter(const std::string& key)
    {
        auto it = headers_.find(key);
        if (it != headers_.end())
        {
            return headers_[key]; //后续调整 目前存在bug 先做个样子
        }
    }

private:
    std::string method_;
    std::string uri_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_; //?body 是否可以放在headers_里面 后续考虑
};