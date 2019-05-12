#ifndef _NET_POLLER_H_
#define _NET_POLLER_H_

#include <sys/epoll.h>
#include <map>
#include <vector>

namespace maou
{
class EventLoop;
class Eventor;

class Poller
{
public:
    static const int EPOLL_EVENT_SIZE = 1024;

    enum
    {
        POLLNONE = 0,
        POLLIN = EPOLLIN,
        POLLOUT = EPOLLOUT,
        POLLERR = EPOLLERR,
        POLLHUB = EPOLLHUP,
    };

    Poller(EventLoop *event_loop);
    ~Poller();

    bool UpdateEvents(Eventor *eventor);
    bool RemoveEvents(Eventor *eventor);

    void Poll(int timeout_ms, std::vector<Eventor *> &active_eventors);

private:
    bool EpollOperate(int operation, Eventor *eventor);

private:
    EventLoop *m_event_loop;
    int m_epoll_fd;
    struct epoll_event m_epoll_events[EPOLL_EVENT_SIZE];
    std::map<int, Eventor *> m_eventors;
};

};  // namespace maou

#endif
