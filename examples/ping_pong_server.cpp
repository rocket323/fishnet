#include <stdio.h>
#include "tcp/tcp_server.h"

void OnNewConn(TcpConnectionPtr conn)
{
    printf("new connection: %s\n", conn->PeerAddr().IpPort().c_str());
}

void OnCloseConn(TcpConnectionPtr conn)
{
    printf("connection closed %s\n", conn->PeerAddr().IpPort().c_str());
}

void OnMsg(TcpConnectionPtr conn, Buffer *buffer)
{
    std::string msg = buffer->RetrieveAllToString();
    printf("connection %s recv msg: %zu\n", conn->PeerAddr().IpPort().c_str(), msg.length());
    conn->Write(msg);
}

int main(int argc, char **argv)
{
    int port = 12345;
    if (argc >= 2)
        port = atoi(argv[1]);

    TcpServer server(EventLoop::Current(), InetAddr(port));
    server.SetConnectionCallback(OnNewConn);
    server.SetCloseCallback(OnCloseConn);
    server.SetReadCallback(OnMsg);

    server.Start();
    EventLoop::Current()->Loop();

    return 0;
}
