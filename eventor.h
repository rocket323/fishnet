#ifndef _NET_EVENTOR_H_
#define _NET_EVENTOR_H_

#include <functional>
#include <memory>

#include "callbacks.h"
#include "poller.h"

class EventLoop;

class Eventor
{
public:
    Eventor(EventLoop *event_loop, int fd);
    ~Eventor();

    void SetEventsCallback(const EventsCallback &cb) { m_events_callback = cb; }
    void HandleEvents();
    void EnableReading();
    void EnableWriting();
    void DisableReading();
    void DisableWriting();
    void DiableAll();

    bool Reading() const { return m_events & Poller::POLLIN; }
    bool Writing() const { return m_events & Poller::POLLOUT; }

    int Fd() const { return m_fd; }
    uint32_t Events() const { return m_events; }
    void SetRevents(uint32_t revents) { m_revents = revents; }

    void Remove();

private:
    void Update();

private:
    EventLoop *m_event_loop;

    const int m_fd;
    uint32_t m_events;
    uint32_t m_revents;

    EventsCallback m_events_callback;
};

#endif
