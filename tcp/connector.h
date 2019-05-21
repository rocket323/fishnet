#ifndef _NET_CONNECTOR_H_
#define _NET_CONNECTOR_H_

#include <memory>
#include "callbacks.h"
#include "connector.h"
#include "event_loop.h"
#include "inet_addr.h"
#include "tcp_connection.h"

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
