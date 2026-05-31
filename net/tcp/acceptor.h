#ifndef _NET_ACCEPTOR_H_
#define _NET_ACCEPTOR_H_

#include <memory>
#include "callbacks.h"
#include "inet_addr.h"

class EventLoop;
class Eventor;
class Socket;

class Acceptor {
public:
    Acceptor(EventLoop *event_loop, const InetAddr &listen_addr,
             const AcceptCallback &accept_callback);
    ~Acceptor();

    bool Listen();
    void Stop();

    const std::string &ErrMsg() const { return err_msg_; }

private:
    void HandleEvents(int revents);
    void HandleRead();

private:
    EventLoop *event_loop_;
    std::unique_ptr<Socket> sock_;
    std::unique_ptr<Eventor> eventor_;
    InetAddr listen_addr_;
    AcceptCallback accept_callback_;
    std::string err_msg_;
};

#endif
