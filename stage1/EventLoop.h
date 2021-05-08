#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "base/Thread.h"
#include "timer.h"

class Channel;
class Epoller;
class TimerQueue;

class EventLoop : noncopyable{
public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void updateChannel(Channel* channel);

    int64_t pollReturnTime() const { return pollReturnTime_; }

    TimerId runAt(const int64_t& time, const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval, const TimerCallback& cb);

    void assertInLoopThread(){
        if(!isInLoopThread()) abortNotInLoopThread();
    }

    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void abortNotInLoopThread();

    typedef std::vector<Channel*> ChannelList;

    bool looping_;
    bool quit_;
    const pid_t threadId_;
    int64_t pollReturnTime_;
    std::shared_ptr<Epoller> poller_;
    std::shared_ptr<TimerQueue> timerQueue_;
    ChannelList activeChannels_;
};