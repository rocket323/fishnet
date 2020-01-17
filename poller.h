#ifndef _NET_POLLER_H_
#define _NET_POLLER_H_

#include <sys/epoll.h>
#include <map>
#include <vector>

class EventLoop;
class Eventor;

class Poller
{
public:
    // File Event Flags
    enum EventFlag
    {
        NONE = 0,
        READABLE = 1 << 0,
        WRITABLE = 1 << 1,
    };

    Poller(EventLoop *event_loop);
    virtual ~Poller();

    static Poller *NewDefaultPoller(EventLoop *event_loop);

    // Poll the I/O events.
    virtual void Poll(int timeout_ms, std::vector<Eventor *> &active_eventors) = 0;

    // Change the interested events.
    virtual bool UpdateEvents(Eventor *eventor) = 0;

    // Remove the events.
    virtual bool RemoveEvents(Eventor *eventor) = 0;

protected:
    std::map<int, Eventor *> eventors_;

private:
    EventLoop *event_loop_;
};

#endif
