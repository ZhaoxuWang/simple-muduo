#include "Channel.h"
#include "EventLoop.h"
#include <poll.h>
#include <assert.h>


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fdArg)
    : loop_(loop), fd_(fdArg), events_(0), revents_(0), index_(-1) {}

Channel::~Channel(){
    assert(!eventHandling_);
}

void Channel::update(){
    loop_->updateChannel(this);
}

void Channel::handleEvent(int64_t receiveTime){
    eventHandling_ = true;
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){
        printf("from EPOLLHUP");
        if(closeCallback_) closeCallback_();
    }
    if(revents_ & EPOLLERR){
        if(errorCallback_) errorCallback_();
    }
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        if(readCallback_) readCallback_(receiveTime);
    }
    if(revents_ & EPOLLOUT){
        if(writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}