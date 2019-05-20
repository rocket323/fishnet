#ifndef _NET_CONNECTOR_H_
#define _NET_CONNECTOR_H_

#include <memory>

class InetAddr;
class EventLoop;
class TcpConnection;

class Connector
{
public:
    static int Connect(const InetAddr &server_addr, int &sockfd);
    static std::shared_ptr<TcpConnection> Connect(EventLoop *event_loop, const InetAddr &server_addr);
};

#endif
