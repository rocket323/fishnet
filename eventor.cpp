#include "eventor.h"
#include "event_loop.h"
#include "log/log.h"

namespace maou
{
Eventor::Eventor(EventLoop *event_loop, int fd) : m_event_loop(event_loop), m_fd(fd), m_events(0), m_revents(0) {}

Eventor::~Eventor() {}

void Eventor::EnableReading()
{
    m_events |= Poller::POLLIN;
    Update();
}

void Eventor::EnableWriting()
{
    m_events |= Poller::POLLOUT;
    Update();
}

void Eventor::DisableReading()
{
    m_events &= ~Poller::POLLIN;
    Update();
}

void Eventor::DisableWriting()
{
    m_events &= ~Poller::POLLOUT;
    Update();
}

void Eventor::DiableAll()
{
    m_events = Poller::POLLNONE;
    Update();
}

void Eventor::RemoveSelf()
{
    m_events = 0;
    m_event_loop->RemoveEvents(this);
}

void Eventor::Update()
{
    m_event_loop->UpdateEvents(this);
}

void Eventor::HandleEvents()
{
    uint32_t revents = m_revents;
    m_revents = 0;
    m_events_callback(revents);
}
}  // namespace maou
