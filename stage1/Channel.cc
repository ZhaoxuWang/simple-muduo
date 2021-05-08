#include "Channel.h"
#include "EventLoop.h"
#include <poll.h>


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fdArg)
    : loop_(loop), fd_(fdArg), events_(0), revents_(0), index_(-1) {}

void Channel::update(){
    loop_->updateChannel(this);
}

void Channel::handleEvent(){
    if(revents_ & EPOLLERR){
        if(errorCallback_) errorCallback_();
    }
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        if(readCallback_) readCallback_();
    }
    if(revents_ & EPOLLOUT){
        if(writeCallback_) writeCallback_();
    }
}