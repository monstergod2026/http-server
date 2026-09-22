#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
class HTTPResponse
{
public:
    HTTPResponse() {}
    static std::string Response()
    {
        return "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: 12\r\nConnection: "
               "close\r\n\r\nHello World!"; //先这样测试用
    }

private:
    std::string method_;
    std::string code_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
};