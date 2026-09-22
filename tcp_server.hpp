#pragma once
#include "acceptor.hpp"
#include "tcp_connection.hpp"
#include <unordered_map>
using MessageCallback = std::function<void(TcpConnection* conn, const std::string& msg)>;

class TcpServer
{
public:
    TcpServer(int port, EventLoop* loop)
        : loop_(loop) //智能指针问题
        , acceptor_(std::make_unique<Acceptor>(port, loop_))
    {
        acceptor_->SetNewConnectionCallback(
            [this](int fd)
            {
                TcpConnection* client_connection = new TcpConnection(fd, this->loop_);
                client_connection->SetCloseCallback(
                    [this, client_connection]()
                    {
                        this->loop_->DeleteChannel(client_connection->channel());
                        client_connection->OnClose();
                    });

                client_connection->SetMessageCallback(
                    [this, client_connection](const char* buffer, ssize_t bytesRead)
                    {
                        std::cout.write(buffer, bytesRead);

                        this->message_callback_(client_connection, std::string(buffer, bytesRead));
                    });
                client_connection->SetOnConnection([]() { LogMessage("连接建立"); });
                client_connection->SetOnClose(
                    [client_connection, this]()
                    {
                        int fd = client_connection->fd();
                        this->loop_->SetRemoveCallback(
                            [fd, this]()
                            {
                                auto it = this->tcp_conns_.find(fd);
                                if (it != tcp_conns_.end())
                                {
                                    delete it->second;
                                    this->tcp_conns_.erase(it); // 从 map 移除
                                }
                            });
                    });
                //注册

                client_connection->OnConnection();
                client_connection->SetMessageBoundary(
                    [](std::string& input_buffer)
                    {
                        std::string request;

                        size_t pos = input_buffer.find("\r\n\r\n");
                        if (pos == std::string::npos)
                        {
                            return request;
                        }
                        request.append(input_buffer, 0, pos + 4);
                        input_buffer.erase(0, pos + 4);
                        return request;
                    });
                this->loop_->AddChannel(client_connection->channel()); //先这样
                this->tcp_conns_[fd] = client_connection;
            }

        );
    }

    void Start() { loop_->Loop(); }
    void SetMessageCallback(MessageCallback message_callback) { message_callback_ = message_callback; }
    ~TcpServer()
    {
        int i = 0;
        for (auto& [fd, conn] : tcp_conns_)
        {
            i++;
            loop_->RemoveChannel(conn->channel()); // 直接 epoll_ctl DEL，不排队
            delete conn;
        }
        std::string result = "退出了" + std::to_string(i) + "条活跃连接";
        LogMessage(result);
    }

private:
    EventLoop* loop_;
    std::unique_ptr<Acceptor> acceptor_;
    std::unordered_map<int, TcpConnection*> tcp_conns_;
    MessageCallback message_callback_;
};