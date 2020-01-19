#include <stdio.h>
#include "tcp/connector.h"
#include "timer_queue.h"

TimerId timer_id = 0;

void OnMsg(TcpConnectionPtr conn, Buffer *buffer) {
    std::string msg = buffer->RetrieveAllToString();
    printf("connection %s recv msg: %zu\n", conn->PeerAddr().IpPort().c_str(), msg.length());
    timer_id = EventLoop::Current()->RunPeriodic(1000, [conn] { conn->Write("Hello world"); });
}

void OnConnected(TcpConnectionPtr conn) {
    printf("connection %s connected\n", conn->PeerAddr().IpPort().c_str());
    conn->SetReadCallback(OnMsg);
    conn->Write("Hello world");
}

void OnClose(TcpConnectionPtr conn) {
    printf("connection %s closed\n", conn->PeerAddr().IpPort().c_str());
    EventLoop::Current()->CancelTimer(timer_id);
}

int main(int argc, char **argv) {
    int port = 12345;
    if (argc >= 2)
        port = atoi(argv[1]);

    auto conn = Connector::Connect(EventLoop::Current(), InetAddr(port));
    conn->SetConnectionCallback(OnConnected);
    conn->SetReadCallback(OnMsg);
    conn->SetCloseCallback(OnClose);

    EventLoop::Current()->Loop();
    return 0;
}
