#pragma once
#include <functional>
#include <sys/time.h>
#include <set>
#include <vector>

#include "base/noncopyable.h"
#include "base/Mutex.h"
#include "Channel.h"

class EventLoop;

typedef std::function<void()> TimerCallback;

class Timer : noncopyable{
public:
    Timer(const TimerCallback& cb, int64_t when, double interval)
    : callback_(cb), expiration_(when), interval_(interval), repeat_(interval > 0.0) {}

    void run() const{
        callback_();
    }

    int64_t expiration() const {return expiration_;}
    bool repeat() const {return repeat_;}

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
};

class TimerId{
public:
    explicit TimerId(Timer* timer) : value_(timer) {}

private:
    Timer* value_;
};

class TimerQueue : noncopyable{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback& cb, int64_t when, double interval);

private:
    typedef std::pair<int64_t, Timer *> Entry;
    typedef std::set <Entry> TimerList;

    void addTimerInLoop(Timer* timer);
    void handleRead();
    std::vector<Entry> getExpired(int64_t now);
    void reset(const std::vector<Entry>& expired, int64_t now);

    bool insert(Timer* timer);
    
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;
};