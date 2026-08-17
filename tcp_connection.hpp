#pragma once
#include "channel.hpp"
using my_server::Channel;
#include "event_loop.hpp"
#include "http_request.hpp"
#include <cstring>
#include <functional>
#include <memory>
using DataCallback = std::function<void(const char*, ssize_t)>;
using EventCallback = std::function<void()>;
using MessageBoundary = std::function<std::string(std::string&)>;
using WriteCompleteCallback = std::function<void()>;
class TcpConnection
{
public:
    TcpConnection(int fd, EventLoop* loop)
        : fd_(fd)
        , client_channel_(new Channel(fd))
        , loop_(loop)
    {
        // message_boundary_ = [this]()
        // {
        //     size_t pos = input_buffer_.find('\n');
        //     if (pos == std::string::npos)
        //         return string{};
        //     size_t len = (pos > 0 && input_buffer_[pos - 1] == '\r') ? pos - 1 : pos;

        //     std::string line = input_buffer_.substr(0, len);
        //     input_buffer_.erase(0, pos + 1);
        //     return line;
        // };

        // message_boundary_ = [this](std::string& input_buffer)
        // {
        //     if (input_buffer.size() < 4)
        //     {
        //         return std::pair<bool, std::string>{false, std::string{}};
        //     }
        //     uint32_t body_len;
        //     memcpy(&body_len, input_buffer.data(), 4);
        //     body_len = ntohl(body_len);
        //     if (body_len + 4 > input_buffer.size())
        //     {
        //         return std::pair<bool, std::string>{false, std::string{}};
        //     }

        //     std::string line = input_buffer.substr(4, body_len);
        //     input_buffer.erase(0, 4 + body_len);
        //     return std::pair<bool, std::string>{true, line};
        // };
        client_channel_->SetReadCallback(
            [this]()
            {
                int fd = this->client_channel_->fd();
                char buffer[1024] = {0};
                ssize_t bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_read < 0)
                {
                    if (!(errno == EAGAIN || errno == EWOULDBLOCK))
                    {
                        // close_callback_();
                        this->Close();
                        return;
                    }
                }
                else if (bytes_read == 0)
                {
                    // 客户端关闭连接
                    LogMessage("Client disconnected");
                    if (!input_buffer_.empty())
                    {
                        while (true)
                        {
                            std ::string line_ = message_boundary_(input_buffer_);
                            if (line_.empty())
                            {
                                LogMessage("残包被丢弃");
                                break;
                            }
                            message_callback_(line_.data(), line_.size());
                        }

                        input_buffer_.clear();
                    }

                    this->Close();
                    return;
                }
                else
                {

                    input_buffer_.append(buffer, bytes_read); // 只做追加
                    if (input_buffer_.size() > 65536)
                    {
                        // close_callback_();
                        this->Close();
                        return;
                    }
                    while (true)
                    {
                        std ::string line_ = message_boundary_(input_buffer_);
                        if (line_.empty())
                        {
                            break;
                        }
                        HTTPRequest request(line_);

                        message_callback_(line_.data(), line_.size());
                    }
                }
            });
        client_channel_->SetWriteCallback([this]() { this->SendMessage(); });
        client_channel_->SetErrorCallback([this]() { this->Close(); });
    }
    void SetMessageBoundary(MessageBoundary cb) { message_boundary_ = cb; }
    void Send(const std::string& response)
    {
        if (output_buffer_.size() > 65535)
        {
            this->Close();
            return;
        }
        if (output_buffer_.size() > 0)
        {
            output_buffer_ += response;
        }
        else
        {
            output_buffer_ += response;
            SendMessage();
        }
    }
    void SendMessage()
    {

        int len = send(fd_, output_buffer_.c_str(), output_buffer_.length(), MSG_NOSIGNAL);
        if (len < 0)
        {
            if (!(errno == EAGAIN || errno == EWOULDBLOCK))
            {
                // close_callback_();
                this->Close();
                return;
            }
            client_channel_->EnableWriting();
            loop_->UpdateChannel(client_channel_);
        }
        else if ((size_t)len < output_buffer_.length())
        {

            LogMessage("Partial send: sent " + std::to_string(len) + " of " + std::to_string(output_buffer_.length()));
            output_buffer_ = output_buffer_.substr(len);
            client_channel_->EnableWriting();
            loop_->UpdateChannel(client_channel_);
        }
        else
        {
            output_buffer_.clear();
            client_channel_->DisableWriting();
            loop_->UpdateChannel(client_channel_);
            if (write_complete_callback_)
                write_complete_callback_();
        }
    }
    int fd() { return fd_; }
    Channel* channel() { return client_channel_; }
    void SetMessageCallback(DataCallback cb) { message_callback_ = cb; }
    void SetCloseCallback(EventCallback cb) { close_callback_ = cb; }
    void SetOnConnection(EventCallback cb) { on_connection_ = cb; }
    void SetOnClose(EventCallback cb) { on_close_ = cb; }
    void SetWriteCompleteCallback(WriteCompleteCallback cb) { write_complete_callback_ = cb; }
    void OnClose()
    {
        if (on_close_)
        {
            on_close_();
        }
    }

    void Close()
    {
        if (closed_)
            return;
        closed_ = true;
        if (close_callback_)
            close_callback_();
    }
    void OnConnection()
    {
        if (on_connection_)
        {
            on_connection_();
        }
    }
    ~TcpConnection()
    {
        delete client_channel_;
        close(fd_);
    }
    // private
private:
    int fd_;
    Channel* client_channel_;
    EventLoop* loop_;
    std::string output_buffer_;
    std::string input_buffer_;
    bool closed_ = false;
    DataCallback message_callback_;
    EventCallback close_callback_;
    EventCallback on_connection_;
    EventCallback on_close_;
    MessageBoundary message_boundary_;
    WriteCompleteCallback write_complete_callback_;
};