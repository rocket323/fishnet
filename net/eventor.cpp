#include "eventor.h"
#include "event_loop.h"

Eventor::Eventor(EventLoop *event_loop, int fd)
    : event_loop_(event_loop), fd_(fd), interest_events_(0), polled_events_(0) {
}

Eventor::~Eventor() {
}

void Eventor::EnableReading() {
    interest_events_ |= Poller::READABLE;
    Update();
}

void Eventor::EnableWriting() {
    interest_events_ |= Poller::WRITABLE;
    Update();
}

void Eventor::DisableReading() {
    interest_events_ &= ~Poller::READABLE;
    Update();
}

void Eventor::DisableWriting() {
    interest_events_ &= ~Poller::WRITABLE;
    Update();
}

void Eventor::DiableAll() {
    interest_events_ = Poller::NONE;
    Update();
}

void Eventor::Remove() {
    interest_events_ = Poller::NONE;
    event_loop_->RemoveEvents(this);
}

void Eventor::Update() {
    event_loop_->UpdateEvents(this);
}

void Eventor::HandleEvents() {
    uint32_t polled_events = polled_events_;
    polled_events_ = Poller::NONE;
    events_callback_(polled_events);
}
