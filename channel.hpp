#pragma once
#include "err.hpp"
#include "log.hpp"
#include <functional>
#include <sys/epoll.h>

namespace my_server
{
using WriteCallback = std::function<void()>;
using ReadCallback = std::function<void()>;
using ErrorCallback = std::function<void()>;
class Channel
{
public:
    Channel(int fd)
        : fd_(fd)
        , events_(EPOLLIN)
        , revents_(0)
        , is_registered_(false)
        , read_callback_(nullptr)
        , write_callback_(nullptr)
        , error_callback_(nullptr)
    {
    }
    ~Channel() = default;

    void SetReadCallback(ReadCallback cb) { read_callback_ = std::move(cb); }
    void SetWriteCallback(WriteCallback cb) { write_callback_ = std::move(cb); }
    void SetErrorCallback(ErrorCallback cb) { error_callback_ = std::move(cb); }
    void SetRevents(int revents) { revents_ = revents; }
    void SetRegistered(bool registered) { is_registered_ = registered; }

    // void DisableReading();

    bool IsRegistered() const { return is_registered_; }
    void EnableWriting() { events_ |= EPOLLOUT; }
    void DisableWriting() { events_ &= ~EPOLLOUT; }

    void HandleEvent()
    {
        // switch(revents_)
        // {}

        if (revents_ & EPOLLIN)
        {
            if (read_callback_)
                read_callback_();
        }
        if (revents_ & EPOLLOUT)
        {
            if (write_callback_)
            {
                write_callback_();
            }
        }
        if (revents_ & EPOLLERR)
        {
            if (error_callback_)
                error_callback_();
        }
        if (revents_ & EPOLLHUP)
        {
            if (error_callback_)
                error_callback_();
        }
        // 后续可以扩展处理 EPOLLOUT、EPOLLERR 等
    }

    int events() const { return events_; }
    int fd() const { return fd_; }

private:
    int fd_;
    int events_;
    int revents_;
    bool is_registered_;
    ReadCallback read_callback_;
    WriteCallback write_callback_;
    ErrorCallback error_callback_;
};
}; // namespace my_server