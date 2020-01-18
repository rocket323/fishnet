#ifndef _NET_POLLER_H_
#define _NET_POLLER_H_

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
    ~Poller();

    // Poll the I/O events.
    void Poll(int timeout_ms, std::vector<Eventor *> &active_eventors);

    // Change the interested events.
    bool UpdateEvents(Eventor *eventor);

    // Remove the events.
    bool RemoveEvents(Eventor *eventor);

private:
    EventLoop *event_loop_;
    std::map<int, Eventor *> eventors_;
    void *api_data_;
};

#endif
