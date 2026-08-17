#include "acceptor.hpp"
#include "event_loop.hpp"
#include "tcp_server.hpp"
#include <memory>
#include <signal.h>



namespace
{
EventLoop* g_main_loop = nullptr;
}
extern "C" void HandleSigInt(int)
{
    if (g_main_loop)
        g_main_loop->Quit();
}
int main()
{
    std::unique_ptr<EventLoop> loop = std::make_unique<EventLoop>();
    TcpServer server(8080, loop.get());
    server.SetMessageCallback([](TcpConnection* conn, const std::string& msg)
                              { conn->Send("server received: " + msg); });
    g_main_loop = loop.get();
    signal(SIGINT, HandleSigInt);

    server.Start();
}