#ifndef _NET_EPOLL_POLLER_H_
#define _NET_EPOLL_POLLER_H_

#include <sys/epoll.h>
#include "poller.h"

class EpollPoller : public Poller
{
public:
    static const int EPOLL_EVENT_SIZE = 1024;

    EpollPoller(EventLoop *event_loop);
    ~EpollPoller();

    void Poll(int timeout_ms, std::vector<Eventor *> &eventors) override;
    bool UpdateEvents(Eventor *eventor) override;
    bool RemoveEvents(Eventor *eventor) override;

private:
    bool EpollOperate(int op, Eventor *eventor);

private:
    int epoll_fd_;
    struct epoll_event epoll_events_[EPOLL_EVENT_SIZE];
};

#endif
