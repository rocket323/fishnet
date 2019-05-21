#ifndef _NET_TCP_CLIENT_H_
#define _NET_TCP_CLIENT_H_

#include "callbacks.h"
#include "connector.h"
#include "event_loop.h"
#include "inet_addr.h"
#include "tcp_connection.h"

class TcpClient
{
public:
    TcpClient(EventLoop *event_loop, const InetAddr &server_addr);
    ~TcpClient();

    bool Connect(const ConnectionCallback &cb);

    const std::string &ErrMsg() const { return err_msg_; }

private:
    EventLoop *event_loop_;
    std::unique_ptr<Connector> connector_;
    TcpConnectionPtr conn_;
    const InetAddr server_addr_;
    std::string err_msg_;
};

#endif
