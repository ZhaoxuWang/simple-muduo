#pragma once
#include <functional>
#include <sys/time.h>
#include <set>
#include <vector>

#include "base/noncopyable.h"
#include "base/Mutex.h"
#include "base/Atomic.h"
#include "Channel.h"

class EventLoop;

typedef std::function<void()> TimerCallback;

class Timer : noncopyable{
public:
    Timer(const TimerCallback& cb, int64_t when, double interval)
    :callback_(cb),
     expiration_(when),
     interval_(interval),
     repeat_(interval > 0.0),
     sequence_(s_numCreated_.incrementAndGet()) { }

    void run() const{
        callback_();
    }

    int64_t expiration() const {return expiration_;}
    bool repeat() const {return repeat_;}
    int64_t sequence() const { return sequence_; }

    void restart(int64_t now){
        if(repeat_){
            expiration_ = now + interval_ * 1000000;
        }
        else{
            expiration_ = 0;
        }
    }

private:
    const TimerCallback callback_;  // 定时器回调函数
    int64_t expiration_;            // 下一次超时时间
    const double interval_;         // 超时时间间隔，如果是一次性定时器，该值为0
    const bool repeat_;             // 是否重复
    const int64_t sequence_;

    static AtomicInt64 s_numCreated_;
};

class TimerId{
public:
    explicit TimerId(Timer* timer = NULL, int64_t seq = 0) : timer_(timer), seq_(seq) {}

    friend class TimerQueue;

private:
    Timer* timer_;
    int64_t seq_;
};

class TimerQueue : noncopyable{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback& cb, int64_t when, double interval);
    void cancel(TimerId timerId);

private:
    typedef std::pair<int64_t, Timer *> Entry;
    typedef std::set <Entry> TimerList;
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    void handleRead();
    std::vector<Entry> getExpired(int64_t now);
    void reset(const std::vector<Entry>& expired, int64_t now);

    bool insert(Timer* timer);
    
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;

    // for cancel()
    bool callingExpiredTimers_;
    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;
};