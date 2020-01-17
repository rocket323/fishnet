#ifndef _NET_SELECT_POLLER_H_
#define _NET_SELECT_POLLER_H_

#include <sys/select.h>
#include "poller.h"

class SelectPoller : public Poller
{
public:
    SelectPoller(EventLoop *event_loop);
    ~SelectPoller();

    void Poll(int timeout_ms, std::vector<Eventor *> &eventors) override;
    bool UpdateEvents(Eventor *eventor) override;
    bool RemoveEvents(Eventor *eventor) override;

private:
    int max_fd_;

    // Interest fd sets.
    fd_set read_fds_;
    fd_set write_fds_;

    // Tmp fd sets.
    fd_set tmp_read_fds_;
    fd_set tmp_write_fds_;
    fd_set tmp_err_fds_;
};

#endif
