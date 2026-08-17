#pragma once
enum ErrorCode
{
    SOCKET_FCNTL_ERROR = 1,
    SOCKET_CREATE_ERROR,
    SOCKET_BIND_ERROR,
    SOCKET_LISTEN_ERROR,
    SOCKET_ACCEPT_ERROR,
    SOCKET_OPT_ERROR,
    EPOLL_CREATE_ERROR,
    EPOLL_CTL_ERROR,
    EPOLL_WAIT_ERROR,
};

enum LogLevel
{
    Debug = 0,
    Info,
    Warn,
    Error,
    Fatal
};