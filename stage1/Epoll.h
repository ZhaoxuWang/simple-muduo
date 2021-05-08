#pragma once
#include <sys/epoll.h>
#include <memory>
#include <vector>
#include <map>
#include "EventLoop.h"

struct epoll_event;

class Channel;

class Epoller : noncopyable{
public:
    typedef std::vector<Channel*> ChannelList;

    Epoller(EventLoop* loop);
    ~Epoller();
    int64_t poll(int timeoutMs, ChannelList* activeChannels);

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

private:
    static const int kInitEventListSize = 16;
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);
    
    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    int epollfd_;
    EventList events_;
    ChannelMap channels_;
};