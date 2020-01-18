#ifndef _EVENT_LOOP_H_
#define _EVENT_LOOP_H_

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#define NET_OK 0
#define NET_ERR -1

class TimerQueue;
class Eventor;
class Poller;

typedef uint64_t TimerId;
typedef std::function<void()> Task;

class EventLoop
{
public:
    ~EventLoop();

    // get the thread-local EventLoop instance for current thread
    // it's thread-locate and thus thread-safe
    static EventLoop *Current();

    // this instance is the one runs in the current thread?
    // it's thread-safe
    inline bool IsCurrent() const
    {
        // current() might not initialized(the thread calls is_current()
        // could has no EventLoop instance constructed)
        return m_thread_id == std::this_thread::get_id();
    }

    void AssertIsCurrent() const
    {
        if (!IsCurrent())
            abort();
    }

    // dispatch will try to run the task we are now
    // int 'loop' thread who owns this EventLoop instance.
    // otherwise, the task is pushed into task queue
    // and wait the 'loop' thread to consume them.
    // they are thread-safe
    inline void Dispatch(const Task &task)
    {
        if (IsCurrent())
            task();
        else
            Post(task);
    }

    inline void Dispatch(Task &&task)
    {
        if (IsCurrent())
            task();
        else
            Post(std::move(task));
    }

    // just post the task into task queue.
    // never try to run the task directly
    // they are thread-safe
    void Post(const Task &task);

    void WakeUp();

    void Loop();
    int LoopOnce(int poll_timeout_ms);
    void Stop();

    // wrap for poller
    void UpdateEvents(Eventor *e);
    void RemoveEvents(Eventor *e);

    // wrap for timer queue
    TimerId RunAt(int64_t expiration, const Task &task);
    TimerId RunAfter(int64_t delay, const Task &task);
    TimerId RunPeriodic(int64_t interval, const Task &task);
    TimerId RunPeriodic(int64_t delay, int64_t interval, const Task &task);
    void CancelTimer(TimerId id);
    int TimerCount() const;

private:
    void HandleEvents(int revents);
    EventLoop();

    // noncopyable
    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

private:
    // thread id
    std::thread::id m_thread_id;

    // poller
    std::mutex m_poller_mutex;
    std::unique_ptr<Poller> m_poller;

    // timer queue
    std::unique_ptr<TimerQueue> m_timer_queue;

    // task
    std::mutex m_tasks_mutex;
    std::vector<Task> m_tasks;

    // running flag
    bool m_running;
};

#endif
