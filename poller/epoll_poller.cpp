#include "epoll_poller.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include "event_loop.h"
#include "eventor.h"

static const int EPOLL_EVENT_SIZE = 1024;

struct PollerApiState
{
    int epoll_fd_ = -1;
    struct epoll_event epoll_events_[EPOLL_EVENT_SIZE];
};

EpollPoller::EpollPoller(EventLoop *event_loop) : event_loop_(event_loop), epoll_fd_(::epoll_create(EPOLL_EVENT_SIZE))
{
    PollerApiState *state = new PollerApiState;
    state->epoll_fd_ = epoll_create(EPOLL_EVENT_SIZE);
    assert(state->epoll_fd_ >= 0);
    api_data_ = state;
}

EpollPoller::~EpollPoller()
{
    PollerApiData *state = static_cast<PollerApiData *>(api_data_);
    close(state->epoll_fd_);

    delete (PollerApiData *)api_data_;
}

bool EpollPoller::UpdateEvents(Eventor *eventor)
{
    // event_loop_->AssertIsCurrent();
    int op = 0;
    if (eventors_.count(eventor->Fd()))
    {
        op = EPOLL_CTL_MOD;
    }
    else
    {
        op = EPOLL_CTL_ADD;
        eventors_[eventor->Fd()] = eventor;
    }

    return EpollOperate(op, eventor);
}

bool EpollPoller::RemoveEvents(Eventor *eventor)
{
    // event_loop_->AssertIsCurrent();
    auto iter = eventors_.find(eventor->Fd());
    if (iter != eventors_.end())
    {
        eventors_.erase(iter);
        return EpollOperate(EPOLL_CTL_DEL, eventor);
    }
    return true;
}

bool EpollPoller::EpollOperate(int op, Eventor *eventor)
{
    PollerApiData *state = static_cast<PollerApiData *>(api_data_);

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    int mask = eventor->InterestEvents();
    ev.data.ptr = eventor;
    ev.events = 0;

    if (mask & Poller::READABLE)
        ev.events |= EPOLLIN;
    if (mask & Poller::WRITABLE)
        ev.events |= EPOLLOUT;

    if (epoll_ctl(state->epoll_fd_, op, eventor->Fd(), &ev) < 0)
        return false;
    return true;
}

void EpollPoller::Poll(int timeout_ms, std::vector<Eventor *> &eventors)
{
    PollerApiData *state = static_cast<PollerApiData *>(api_data_);

    eventors.clear();
    int num_events = epoll_wait(state->epoll_fd_, state->epoll_events_, EPOLL_EVENT_SIZE, timeout_ms);
    if (num_events < 0)
    {
        if (errno != EINTR)
        {
            // error
        }
        else
        {
            num_events = 0;
        }
    }
    else if (num_events > 0)
    {
        eventors.resize(num_events);
        for (int i = 0; i < num_events; i++)
        {
            int mask = 0;
            auto &e = state->epoll_events_[i];

            if (e.events & EPOLLIN)
                mask |= Poller::READABLE;
            if (e.events & EPOLLOUT)
                mask |= Poller::WRITABLE;
            if (e.events & EPOLLERR)
                mask |= (Poller::READABLE | Poller::WRITABLE);
            if (e.events & EPOLLHUP)
                mask |= (Poller::READABLE | Poller::WRITABLE);

            eventors[i] = static_cast<Eventor *>(e.data.ptr);
            eventors[i]->SetPolledEvents(mask);
        }
    }
}
