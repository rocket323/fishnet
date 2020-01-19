#ifndef _NET_EVENTOR_H_
#define _NET_EVENTOR_H_

#include <functional>
#include <memory>

#include "callbacks.h"
#include "poller.h"

class EventLoop;

class Eventor {
public:
    Eventor(EventLoop *event_loop, int fd);
    ~Eventor();

    void SetEventsCallback(const EventsCallback &cb) { events_callback_ = cb; }
    void HandleEvents();
    void EnableReading();
    void EnableWriting();
    void DisableReading();
    void DisableWriting();
    void DiableAll();

    bool Reading() const { return interest_events_ & Poller::READABLE; }
    bool Writing() const { return interest_events_ & Poller::WRITABLE; }

    int Fd() const { return fd_; }
    uint32_t InterestEvents() const { return interest_events_; }
    void SetPolledEvents(uint32_t polled_events) { polled_events_ = polled_events; }

    void Remove();

private:
    void Update();

private:
    EventLoop *event_loop_;

    const int fd_;

    // Events that eventor interest in.
    uint32_t interest_events_;
    // Events that eventor really happens.
    uint32_t polled_events_;

    EventsCallback events_callback_;
};

#endif
