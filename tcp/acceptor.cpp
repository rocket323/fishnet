#include "acceptor.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "event_loop.h"
#include "eventor.h"
#include "socket.h"

using namespace std::placeholders;

Acceptor::Acceptor(EventLoop *event_loop, const InetAddr &listen_addr, const AcceptCallback &accept_callback)
    : event_loop_(event_loop), sock_(), eventor_(), listen_addr_(listen_addr), accept_callback_(accept_callback)
{
}

Acceptor::~Acceptor()
{
}

bool Acceptor::Listen()
{
    int sockfd = sockets::CreateNonBlockingStreamSocket();
    if (sockfd < 0)
    {
        err_msg_ = std::string(strerror(errno));
        return false;
    }

    sock_.reset(new Socket(sockfd));
    sock_->SetReuseAddr(true);
    if (!sock_->BindAndListen(listen_addr_, 1024))
    {
        // error
        sock_.reset();
        return false;
    }

    eventor_.reset(new Eventor(event_loop_, sockfd));
    eventor_->SetEventsCallback(std::bind(&Acceptor::HandleEvents, this, _1));
    eventor_->EnableReading();
    return true;
}

void Acceptor::Stop()
{
    if (sock_)
    {
        assert(eventor_);
        eventor_->Remove();
        eventor_.reset();
        sock_.reset();
    }
}

void Acceptor::HandleEvents(int revents)
{
    if (revents & Poller::READABLE)
        HandleRead();
}

void Acceptor::HandleRead()
{
    int fd = sock_->Accept();
    if (fd >= 0)
    {
        assert(accept_callback_);
        accept_callback_(fd);
    }
    else
    {
        // error
    }
}
