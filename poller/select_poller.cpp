#include "select_poller.h"
#include <sys/select.h>
#include <sys/time.h>
#include <cstring>
#include <string>
#include "event_loop.h"
#include "eventor.h"

SelectPoller::SelectPoller(EventLoop *event_loop) : Poller(event_loop), max_fd_(-1)
{
}

SelectPoller::~SelectPoller()
{
}

bool SelectPoller::UpdateEvents(Eventor *eventor)
{
    int mask = eventor->InterestEvents();
    // Clear all flags first.
    FD_CLR(eventor->Fd(), &read_fds_);
    FD_CLR(eventor->Fd(), &write_fds_);

    if (mask & Poller::READABLE)
        FD_SET(eventor->Fd(), &read_fds_);
    if (mask & Poller::WRITABLE)
        FD_SET(eventor->Fd(), &write_fds_);

    if (!eventors_.count(eventor->Fd()))
        eventors_[eventor->Fd()] = eventor;

    // Update max_fd_.
    if (eventor->Fd() > max_fd_)
        max_fd_ = eventor->Fd();

    return true;
}

bool SelectPoller::RemoveEvents(Eventor *eventor)
{
    FD_CLR(eventor->Fd(), &read_fds_);
    FD_CLR(eventor->Fd(), &write_fds_);

    eventors_.erase(eventor->Fd());

    // Update max_fd_.
    if (eventor->Fd() == max_fd_)
    {
        max_fd_ = -1;
        for (auto &kv : eventors_)
        {
            if (kv.second->Fd() > max_fd_)
                max_fd_ = kv.second->Fd();
        }
    }

    return true;
}

void SelectPoller::Poll(int timeout_ms, std::vector<Eventor *> &eventors)
{
    eventors.clear();
    if (max_fd_ == -1)
        return;

    memcpy(&tmp_read_fds_, &read_fds_, sizeof(fd_set));
    memcpy(&tmp_write_fds_, &write_fds_, sizeof(fd_set));
    memset(&tmp_err_fds_, 0, sizeof(fd_set));

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int num = ::select(max_fd_ + 1, &tmp_read_fds_, &tmp_write_fds_, &tmp_err_fds_, &tv);
    if (num > 0)
    {
        for (auto &kv : eventors_)
        {
            auto eventor = kv.second;
            if (eventor->InterestEvents() == Poller::NONE)
                continue;

            int mask = 0;
            if (eventor->Reading() && FD_ISSET(eventor->Fd(), &tmp_read_fds_))
                mask |= Poller::READABLE;

            if (eventor->Writing() && FD_ISSET(eventor->Fd(), &tmp_write_fds_))
                mask |= Poller::WRITABLE;

            if (FD_ISSET(eventor->Fd(), &tmp_err_fds_))
                mask |= (Poller::READABLE | Poller::WRITABLE);

            eventor->SetPolledEvents(mask);
            eventors.push_back(eventor);
        }
    }
}
