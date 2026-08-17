#pragma once
#include "comm.hpp"
#include "err.hpp"
#include "log.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

const int kBacklog = 64;
class Sock
{
public:
    Sock(int port = 8080)
        : sock_fd_(-1)
        , port_(port)
    {
        //绑定套接字
        sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd_ < 0)
        {
            LogMessage("Failed to create socket");
            exit(SOCKET_CREATE_ERROR);
        }

        // 设置套接字选项，允许地址重用 ai辅助填写
        int opt = 1;
        if (setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            // 这里打印错误并退出，或者用你的 LogMessage 记录
            LogMessage("Failed to setsockopt");
            exit(SOCKET_OPT_ERROR); // 如果有定义这个错误码的话
        }

        SetNonBlock(sock_fd_); // 设置非阻塞模式

        //绑定端口
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(sock_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        {
            LogMessage("Failed to bind socket");
            exit(SOCKET_BIND_ERROR);
        }
        //监听套接字
        if (listen(sock_fd_, kBacklog) < 0)
        {
            LogMessage("Failed to listen on socket");
            exit(SOCKET_LISTEN_ERROR);
        }
    }
    int Accept()
    {

        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(sock_fd_, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                LogMessage("Failed to accept connection");
            return -1; // 返回 -1 表示接受连接失败
        }
        return client_fd;
    }

    ~Sock()
    {
        if (sock_fd_ != -1)
        {
            close(sock_fd_);
        }
    }

    int sock_fd() const { return sock_fd_; }

private:
    int sock_fd_;
    int port_;
};