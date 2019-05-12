#ifndef _NET_TIMER_QUEUE_H_
#define _NET_TIMER_QUEUE_H_

#include <cstdint>
#include <functional>
#include <map>
#include <set>

namespace maou
{
typedef uint64_t TimerId;
typedef std::function<void()> TimerTask;

struct Timer
{
    TimerId timer_id;
    TimerTask task;
    int64_t expiration_ms;
    int64_t interval_ms;
    bool operator<(const Timer &rhs) const
    {
        if (expiration_ms != rhs.expiration_ms)
            return expiration_ms < rhs.expiration_ms;
        return timer_id < rhs.timer_id;
    }
};

class TimerQueue
{
public:
    TimerId AddTimer(const TimerTask &task, int64_t expiration_ms, int64_t interval_ms);
    void RemoveTimer(TimerId timer_id);

    // next_expiration == -1 when no timers
    int Expire(int64_t now_ms, int64_t &next_expiration);

    int TimerCount() const { return (int)m_timers.size(); }

private:
    static TimerId NextTimerId();

    static TimerId m_next_timer_id;  // atomic

    void AddTimer(const Timer &timer);

private:
    std::set<Timer> m_timers;
    std::map<TimerId, int64_t> m_timer_expiration;
};

}  // namespace maou
#endif
