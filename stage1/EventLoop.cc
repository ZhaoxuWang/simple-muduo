#include "EventLoop.h"
#include "timer.h"
#include "Channel.h"
#include "Epoll.h"
#include "utils.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include <assert.h>

#include <poll.h>

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

EventLoop::EventLoop()
     : looping_(false),
     quit_(false),
     threadId_(CurrentThread::tid()),
     poller_(new Epoller(this)),
     timerQueue_(new TimerQueue(this))
{
    if(t_loopInThisThread){
        printf("Another EventLoop exists in this thread");
    }
    else{
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop(){
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop(){
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while(!quit_){
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for(auto it = activeChannels_.begin(); it != activeChannels_.end(); it++){
            (*it)->handleEvent();
        }
    }

    looping_ = false;
}

void EventLoop::quit(){
    quit_ = true;
}

TimerId EventLoop::runAt(const int64_t& time, const TimerCallback& cb){
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb){
    int64_t time = addTime(get_now(), delay);
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb){
    int64_t time = addTime(get_now(), interval);
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::updateChannel(Channel* channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::abortNotInLoopThread(){
    perror("abortNotInLoopThread");
}