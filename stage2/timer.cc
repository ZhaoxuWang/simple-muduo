#include "timer.h"
#include "EventLoop.h"
#include "utils.h"

#include <sys/timerfd.h>
#include <functional>
#include <assert.h>
#include <string.h>

int createTimerfd(){
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    assert(timerfd > 0);
    return timerfd;
}

struct timespec howMuchTimeFromNow(int64_t when){
    int64_t microseconds = when - get_now();

    if(microseconds < 100){
        microseconds = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / 1000000);
    ts.tv_nsec = static_cast<long>((microseconds % 1000000) * 1000);
    return ts;
}

void readTimerfd(int timerfd, int64_t now){
    uint64_t howmany;
    ssize_t n = read(timerfd, &howmany, sizeof(howmany));
    assert(n == sizeof(howmany));
}

void resetTimerfd(int timerfd, int64_t expiration){
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
    assert(ret == 0);
}

TimerQueue::TimerQueue(EventLoop* loop)
    :loop_(loop),
     timerfd_(createTimerfd()),
     timerfdChannel_(loop, timerfd_),
     timers_()
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue(){
    close(timerfd_);
    for(auto it = timers_.begin(); it != timers_.end(); it++){
        delete it->second;
    }
}

TimerId TimerQueue::addTimer(const TimerCallback& cb, int64_t when, double interval){
    Timer* timer = new Timer(cb, when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer);
}

void TimerQueue::addTimerInLoop(Timer* timer){
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if(earliestChanged){
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void TimerQueue::handleRead(){
    loop_->assertInLoopThread();
    int64_t now = get_now();
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    for(auto it = expired.begin(); it != expired.end(); it++){
        it->second->run();
    }
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(int64_t now){
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, int64_t now){
    int64_t nextExpire = 0;

    for(auto it = expired.begin(); it != expired.end(); it++){
        if(it->second->repeat()){
            it->second->restart(now);
            insert(it->second);
        }
        else{
            delete it->second;
        }
    }

    if(!timers_.empty()){
        nextExpire = timers_.begin()->second->expiration();
    }

    if(nextExpire > 0){
        resetTimerfd(timerfd_, nextExpire);
    }
}

bool TimerQueue::insert(Timer* timer){
    bool earliestChanged = false;
    int64_t when = timer->expiration();
    auto it = timers_.begin();
    if(it == timers_.end() || when < it->first){
        earliestChanged = true;
    }
    auto result = timers_.insert(std::make_pair(when, timer));
    assert(result.second);
    return earliestChanged;
}