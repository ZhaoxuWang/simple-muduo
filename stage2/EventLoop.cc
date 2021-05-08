#include "EventLoop.h"
#include "timer.h"
#include "Channel.h"
#include "Epoll.h"
#include "utils.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <iostream>
#include <assert.h>
#include <functional>
#include <poll.h>
#include <signal.h>

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;

int createEventfd(){
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0){
        perror("Failed in eventfd\n");
        abort();
    }
    return evtfd;
}

// 创建一个全局变量，在这个变量的构造函数中互略SIGPIPE
class IgnoreSigPipe{
public:
    IgnoreSigPipe(){
        signal(SIGPIPE, SIG_IGN);
    }
};
IgnoreSigPipe initObj;

EventLoop::EventLoop()
     : looping_(false),
     quit_(false),
     callingPendingFunctors_(false),
     threadId_(CurrentThread::tid()),
     poller_(new Epoller(this)),
     timerQueue_(new TimerQueue(this)),
     wakeupFd_(createEventfd()),
     wakeupChannel_(new Channel(this, wakeupFd_))
{
    if(t_loopInThisThread){
        printf("Another EventLoop exists in this thread");
    }
    else{
        t_loopInThisThread = this;
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop(){
    assert(!looping_);
    close(wakeupFd_);
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
            (*it)->handleEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }

    looping_ = false;
}

void EventLoop::quit(){
    quit_ = true;
    if(!isInLoopThread()){
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor& cb){
    if(isInLoopThread()){
        cb();
    }
    else{
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb){
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if(!isInLoopThread() || callingPendingFunctors_){
        wakeup();
    }
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

void EventLoop::removeChannel(Channel* channel){
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

void EventLoop::abortNotInLoopThread(){
    perror("abortNotInLoopThread");
}

void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one)){
        perror("EventLoop::wakeup() writed failed");
    }
}

void EventLoop::handleRead(){
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if(n != sizeof(one)){
        perror("EventLoop::handleRead() readed failed");
    }
}

void EventLoop::doPendingFunctors(){
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(int i = 0; i < functors.size(); i++){
        functors[i]();
    }
    callingPendingFunctors_ = false;
}