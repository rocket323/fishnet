#include <sys/select.h>
#include <sys/time.h>
#include <cstring>
#include <string>
#include "event_loop.h"
#include "eventor.h"

struct PollerApiData
{
    int max_fd_ = -1;

    // Interest fd sets.
    fd_set read_fds_;
    fd_set write_fds_;

    // Tmp fd sets.
    fd_set tmp_read_fds_;
    fd_set tmp_write_fds_;
    fd_set tmp_err_fds_;
};

Poller::Poller(EventLoop *event_loop) : event_loop_(event_loop)
{
    api_data_ = new PollerApiData;
}

Poller::~Poller()
{
    delete (PollerApiData *)api_data_;
}

bool Poller::UpdateEvents(Eventor *eventor)
{
    PollerApiData *state = static_cast<PollerApiData *>(api_data_);

    int mask = eventor->InterestEvents();
    // Clear all flags first.
    FD_CLR(eventor->Fd(), &state->read_fds_);
    FD_CLR(eventor->Fd(), &state->write_fds_);

    if (mask & Poller::READABLE)
        FD_SET(eventor->Fd(), &state->read_fds_);
    if (mask & Poller::WRITABLE)
        FD_SET(eventor->Fd(), &state->write_fds_);

    if (!eventors_.count(eventor->Fd()))
        eventors_[eventor->Fd()] = eventor;

    // Update max_fd_.
    if (eventor->Fd() > state->max_fd_)
        state->max_fd_ = eventor->Fd();

    return true;
}

bool Poller::RemoveEvents(Eventor *eventor)
{
    PollerApiData *state = static_cast<PollerApiData *>(api_data_);

    FD_CLR(eventor->Fd(), &state->read_fds_);
    FD_CLR(eventor->Fd(), &state->write_fds_);

    eventors_.erase(eventor->Fd());

    // Update max_fd_.
    if (eventor->Fd() == state->max_fd_)
    {
        state->max_fd_ = -1;
        for (auto &kv : eventors_)
        {
            if (kv.second->Fd() > state->max_fd_)
                state->max_fd_ = kv.second->Fd();
        }
    }

    return true;
}

void Poller::Poll(int timeout_ms, std::vector<Eventor *> &eventors)
{
    PollerApiData *state = static_cast<PollerApiData *>(api_data_);

    eventors.clear();
    if (state->max_fd_ == -1)
        return;

    memcpy(&state->tmp_read_fds_, &state->read_fds_, sizeof(fd_set));
    memcpy(&state->tmp_write_fds_, &state->write_fds_, sizeof(fd_set));
    memset(&state->tmp_err_fds_, 0, sizeof(fd_set));

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int num = select(state->max_fd_ + 1, &state->tmp_read_fds_, &state->tmp_write_fds_, &state->tmp_err_fds_, &tv);
    if (num > 0)
    {
        for (auto &kv : eventors_)
        {
            auto eventor = kv.second;
            if (eventor->InterestEvents() == Poller::NONE)
                continue;

            int mask = 0;
            if (eventor->Reading() && FD_ISSET(eventor->Fd(), &state->tmp_read_fds_))
                mask |= Poller::READABLE;

            if (eventor->Writing() && FD_ISSET(eventor->Fd(), &state->tmp_write_fds_))
                mask |= Poller::WRITABLE;

            if (FD_ISSET(eventor->Fd(), &state->tmp_err_fds_))
                mask |= (Poller::READABLE | Poller::WRITABLE);

            eventor->SetPolledEvents(mask);
            eventors.push_back(eventor);
        }
    }
}
