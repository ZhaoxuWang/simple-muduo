#include "Epoll.h"
#include "Channel.h"
#include "utils.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

Epoller::Epoller(EventLoop* loop)
    : ownerLoop_(loop), epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize){
    assert(epollfd_ > 0);    
}

Epoller::~Epoller(){
    close(epollfd_);
}

int64_t Epoller::poll(int timeoutMs, ChannelList* activeChannels){
    int numEvents = epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeoutMs);
    int64_t now = get_now();

    if(numEvents < 0) perror("epoll wait error");
    else if(numEvents > 0)fillActiveChannels(numEvents, activeChannels);

    return now;
}

void Epoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const{
    for(int i = 0; i < numEvents; i++){
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void Epoller::updateChannel(Channel* channel){
    assertInLoopThread();
    const int index = channel->index();
    if(index == kNew || index == kDeleted){
        int fd = channel->fd();
        if(index == kNew){
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else{
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else{
        if(channel->isNoneEvent()){
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else{
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Epoller::removeChannel(Channel* channel){
    assertInLoopThread();
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    channels_.erase(fd);
    if(index == kAdded){
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void Epoller::update(int operation, Channel* channel){
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if(epoll_ctl(epollfd_, operation, fd, &event) < 0){
        perror("epoll_mod error");
    }
}