#include "event_loop.h"
#include <assert.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <unistd.h>
#include "eventor.h"
#include "poller.h"
#include "timer_queue.h"

using namespace std::placeholders;

static int CreateEventFd()
{
    int event_fd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (event_fd < 0)
        abort();

    return event_fd;
}

int64_t CurrentSystemTimeMillis()
{
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    return static_cast<int64_t>(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

time_t CurrentSystemTime()
{
    time_t now = ::time(NULL);
    return now;
}

EventLoop::EventLoop()
    : m_thread_id(std::this_thread::get_id()),
      m_wakeup_fd(CreateEventFd()),
      m_wakeup_eventor(new Eventor(this, m_wakeup_fd)),
      m_poller(Poller::NewDefaultPoller(this)),
      m_timer_queue(new TimerQueue()),
      m_running(false)
{
    m_wakeup_eventor->SetEventsCallback(std::bind(&EventLoop::HandleEvents, this, _1));
    m_wakeup_eventor->EnableReading();
}

EventLoop::~EventLoop()
{
    ::close(m_wakeup_fd);
}

__thread EventLoop *tls_ptr = NULL;
EventLoop *EventLoop::Current()
{
    if (tls_ptr == NULL)
        tls_ptr = new EventLoop;
    return tls_ptr;
}

void EventLoop::Post(const Task &task)
{
    bool is_empty;
    {
        std::lock_guard<std::mutex> lock(m_tasks_mutex);
        is_empty = m_tasks.empty();
        m_tasks.push_back(task);
    }

    // wake up to run task when this is the first task
    // and not in current loop thread
    if (is_empty && !IsCurrent())
        WakeUp();
}

void EventLoop::UpdateEvents(Eventor *eventor)
{
    std::lock_guard<std::mutex> lock(m_poller_mutex);
    m_poller->UpdateEvents(eventor);
}

void EventLoop::RemoveEvents(Eventor *eventor)
{
    std::lock_guard<std::mutex> lock(m_poller_mutex);
    m_poller->RemoveEvents(eventor);
}

TimerId EventLoop::RunAt(int64_t expiration, const Task &task)
{
    // DCHECK(IsCurrent());
    return m_timer_queue->AddTimer(task, expiration, 0);
}

TimerId EventLoop::RunAfter(int64_t delay, const Task &task)
{
    // DCHECK(IsCurrent());
    return m_timer_queue->AddTimer(task, CurrentSystemTimeMillis() + delay, 0);
}

TimerId EventLoop::RunPeriodic(int64_t interval, const Task &task)
{
    // DCHECK(IsCurrent());
    return m_timer_queue->AddTimer(task, CurrentSystemTimeMillis() + interval, interval);
}

TimerId EventLoop::RunPeriodic(int64_t delay, int64_t interval, const Task &task)
{
    // DCHECK(IsCurrent());
    return m_timer_queue->AddTimer(task, CurrentSystemTimeMillis() + delay, interval);
}

void EventLoop::CancelTimer(TimerId id)
{
    // DCHECK(IsCurrent());
    return m_timer_queue->RemoveTimer(id);
}

int EventLoop::TimerCount() const
{
    return m_timer_queue->TimerCount();
}

void EventLoop::Loop()
{
    AssertIsCurrent();
    m_running = true;
    while (m_running)
    {
        // wait for 5ms at most
        LoopOnce(5);
    }
}

int EventLoop::LoopOnce(int poll_timeout_ms)
{
    // must run in loop-thread
    // DCHECK(IsCurrent());

    int num = 0;

    // tasks
    std::vector<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(m_tasks_mutex);
        tasks.swap(m_tasks);
    }

    for (auto it = tasks.begin(); it != tasks.end(); it++)
        (*it)();
    num += tasks.size();

    // timer queue
    int64_t next_expiration = -1;
    int64_t now_ms = CurrentSystemTimeMillis();
    num += m_timer_queue->Expire(now_ms, next_expiration);

    // poller
    if (next_expiration > now_ms && (poll_timeout_ms == -1 || (next_expiration - now_ms) < poll_timeout_ms))
        poll_timeout_ms = next_expiration - now_ms;

    {
        // poll don't wait if there are pending tasks
        std::lock_guard<std::mutex> lock(m_tasks_mutex);
        if (!m_tasks.empty())
            poll_timeout_ms = 0;
    }

    std::vector<Eventor *> active_eventors;
    m_poller->Poll(poll_timeout_ms, active_eventors);
    for (auto iter = active_eventors.begin(); iter != active_eventors.end(); iter++)
    {
        (*iter)->HandleEvents();
        num++;
    }

    return num;
}

void EventLoop::Stop()
{
    m_running = false;
    if (!IsCurrent())
        WakeUp();
}

void EventLoop::HandleEvents(int revents)
{
    uint64_t dummy = 0;
    ssize_t ret = ::read(m_wakeup_fd, &dummy, sizeof(dummy));
    (void)ret;
}

void EventLoop::WakeUp()
{
    uint64_t one = 1;
    ssize_t ret = ::write(m_wakeup_fd, &one, sizeof(one));
    (void)ret;
}
