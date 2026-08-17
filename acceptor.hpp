
#pragma once
#include "channel.hpp"
using my_server::Channel;
#include "event_loop.hpp"
#include "sock.hpp"
#include <functional>
#include <memory>
using NewConnectionCallback = std::function<void(int)>;

class Acceptor
{
public:
    Acceptor(int port, EventLoop* event_loop)
        : sock_(std::make_unique<Sock>(port))
        , fd_(sock_->sock_fd())
        , listen_channel_(std::make_unique<Channel>(fd_))
        , event_loop_(event_loop)
    {
        listen_channel_->SetReadCallback(
            [this]()
            {
                while (true)
                {
                    int fd = this->sock_->Accept();
                    if (fd < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break; // 正常：队列已空
                        LogMessage("accept error: " + std::to_string(errno));
                        break; // 真错误：记日志退出
                    }
                    SetNonBlock(fd); // 设置新连接为非阻塞模式
                    this->OnAccept(fd);
                }
            }

        );

        event_loop_->AddChannel(listen_channel_.get()); //完成注册
    }
    void SetNewConnectionCallback(NewConnectionCallback cb) { new_connection_callback_ = cb; }
    void OnAccept(int fd)
    {
        if (new_connection_callback_)
        {
            new_connection_callback_(fd);
        }
    }

private:
    std::unique_ptr<Sock> sock_;
    int fd_;
    std::unique_ptr<Channel> listen_channel_;
    EventLoop* event_loop_;
    NewConnectionCallback new_connection_callback_;
};