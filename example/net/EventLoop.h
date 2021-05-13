#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "base/Thread.h"
#include "base/Mutex.h"
#include "timer.h"

class Channel;
class Epoller;
class TimerQueue;

class EventLoop : noncopyable{
public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    int64_t pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);
    void wakeup();


    TimerId runAt(const int64_t& time, const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval, const TimerCallback& cb);

    void cancel(TimerId timerId);

    void assertInLoopThread(){
        if(!isInLoopThread()) abortNotInLoopThread();
    }

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void abortNotInLoopThread();
    void handleRead();  // waked up
    void doPendingFunctors();

    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    bool quit_;
    bool callingPendingFunctors_;
    const pid_t threadId_;
    int64_t pollReturnTime_;
    std::unique_ptr<Epoller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;
    MutexLock mutex_;
    std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_
};