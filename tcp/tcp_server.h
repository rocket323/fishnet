#ifndef _NET_TCP_SERVER_H_
#define _NET_TCP_SERVER_H_

#include <map>
#include <memory>
#include "callbacks.h"
#include "event_loop.h"
#include "inet_addr.h"
#include "tcp_connection.h"

class EventLoop;
class Acceptor;
class TcpConnection;

class TcpServer {
public:
    TcpServer(EventLoop *event_loop, const InetAddr &server_addr);
    ~TcpServer();

    void SetConnectionCallback(const ConnectionCallback &cb) { connection_callback_ = cb; }
    void SetCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }
    void SetReadCallback(const ReadCallback &cb) { read_callback_ = cb; }

    bool Start();
    void Stop();

    const InetAddr &ServerAddr() const { return server_addr_; }
    const std::string &ErrMsg() const { return err_msg_; }

private:
    void OnAccept(int sockfd);
    void RemoveConnection(TcpConnectionPtr conn);

private:
    EventLoop *event_loop_;
    const InetAddr server_addr_;
    std::unique_ptr<Acceptor> acceptor_;
    ConnectionCallback connection_callback_;
    CloseCallback close_callback_;
    ReadCallback read_callback_;
    std::string err_msg_;

    std::map<uint64_t, TcpConnectionPtr> conns_map_;
};

#endif
