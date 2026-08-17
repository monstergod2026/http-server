#pragma once
#include "err.hpp"
#include "log.hpp"
#include <fcntl.h>
void SetNonBlock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        LogMessage("Failed to get fd flags");
        exit(SOCKET_FCNTL_ERROR);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        LogMessage("Failed to set fd non-blocking");
        exit(SOCKET_FCNTL_ERROR);
    }
}