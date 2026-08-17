#pragma once
#include "channel.hpp"
using my_server::Channel;
#include "err.hpp"
#include "log.hpp"
#include <sys/epoll.h>
#include <unistd.h>
const int MAX_EVENTS = 10;
class Poller
{
public:
    Poller()
        : epoll_fd_(epoll_create1(0))
    {
        if (epoll_fd_ == -1)
        {
            LogMessage("Failed to create epoll instance");
            exit(EPOLL_CREATE_ERROR);
        }
    }
    ~Poller() { close(epoll_fd_); }

    void AddChannel(Channel* ch)
    {
        int fd = ch->fd();
        epoll_event event;
        event.events = ch->events();
        event.data.ptr = ch;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) == -1)
        {
            LogMessage("Failed to add fd to epoll");
            exit(EPOLL_CTL_ERROR);
        }
        ch->SetRegistered(true);
    }
    void UpdateChannel(Channel* ch)
    {

        int fd = ch->fd();
        epoll_event event;
        event.events = ch->events();
        event.data.ptr = ch;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) == -1)
        {
            LogMessage("Failed to update fd to epoll");
            exit(EPOLL_CTL_ERROR);
        }
    }
    void RemoveChannel(Channel* ch)
    {
        int fd = ch->fd();
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1)
        {
            LogMessage("Failed to remove fd from epoll");
        }
    }

    // 返回就绪事件数量，就绪事件列表存在内部 events_ 里
    int Poll(int timeout) { return epoll_wait(epoll_fd_, events_, MAX_EVENTS, timeout); }

    epoll_event* events() { return events_; }

private:
    int epoll_fd_;
    epoll_event events_[MAX_EVENTS];
};