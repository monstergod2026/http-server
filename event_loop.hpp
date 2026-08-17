#pragma once
#include "channel.hpp"
using my_server::Channel;
#include "poller.hpp"
#include "sock.hpp"

#include <memory>
#include <sys/epoll.h>
#include <unordered_map>
#include <vector>

#include <atomic>
using RemoveCallback = std::function<void()>;
using PendingFunctor = std::function<void()>;
class EventLoop
{
public:
    EventLoop()
        : poller_(std::make_unique<Poller>())
        , quit_(false)
    {
    }

    ~EventLoop() { RunPending(); }
    void Quit() { quit_.store(true); }
    void HandleEvents(int n)
    {
        for (int i = 0; i < n; ++i)
        {
            Channel* ch = static_cast<Channel*>(poller_->events()[i].data.ptr);
            ch->SetRevents(poller_->events()[i].events);
            ch->HandleEvent();
        }
    }
    void Start(int timeout)
    {
        int n = poller_->Poll(timeout);

        if (n == -1)
        {
            if (errno == EINTR)
            {
                return;
            }
            LogMessage("epoll_wait failed");
            exit(EPOLL_WAIT_ERROR);
        }
        switch (n)
        {
        case 0:
            // 没有事件发生，继续等待
            break;
        case -1:
            // 处理错误或超时
            break;
        default:
            HandleEvents(n);
            break;
        }
    }
    void Loop()
    {
        int timeout = -1; // 无限等待

        while (!quit_.load())
        {
            Start(timeout);
            RunPending();
        }
    }
    // void updateChannel(Channel* channel) {}

    void AddChannel(Channel* ch)
    {
        client_connections_[ch->fd()] = ch;
        poller_->AddChannel(ch);
    }
    void UpdateChannel(Channel* ch) { poller_->UpdateChannel(ch); }
    void RemoveChannel(Channel* ch) { poller_->RemoveChannel(ch); }
    void RunPending()
    {
        for (auto e : pending_removals_)
        {
            int fd = e->fd();
            RemoveChannel(e);
            auto it = client_connections_.find(fd);
            if (it != client_connections_.end())
            {
                client_connections_.erase(it); // 从 map 移除
            }
        }
        pending_removals_.clear();

        for (auto e : pending_functors_)
        {
            e();
        }
        pending_functors_.clear();
    }
    void DeleteChannel(Channel* ch) { pending_removals_.push_back(ch); }
    void SetRemoveCallback(RemoveCallback cb) { pending_functors_.push_back(cb); }

private:
    std::vector<Channel*> pending_removals_;
    std::unordered_map<int, Channel*> client_connections_;

    std::unique_ptr<Poller> poller_;
    // RemoveCallback removecallback_;
    std::vector<PendingFunctor> pending_functors_;
    std::atomic<bool> quit_;
};