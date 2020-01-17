#include "poller.h"
#include "event_loop.h"
#include "eventor.h"
#include "poller/epoll_poller.h"
#include "poller/select_poller.h"

Poller *Poller::NewDefaultPoller(EventLoop *event_loop)
{
    // return new EpollPoller(event_loop);
    return new SelectPoller(event_loop);
}

Poller::Poller(EventLoop *event_loop) : event_loop_(event_loop)
{
}

Poller::~Poller()
{
}
