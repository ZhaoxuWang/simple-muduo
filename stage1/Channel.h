#pragma once
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include "base/noncopyable.h"

class EventLoop;

// 每个Chanell属于一个EventLoop，负责一个fd，需要由EventLoop remove
class Channel : noncopyable{
public:
    typedef std::function<void()> EventCallback;
    Channel(EventLoop* loop, int fd);

    void handleEvent();
    void setReadCallback(const EventCallback& cb) {readCallback_ = cb;}
    void setWriteCallback(const EventCallback& cb) {writeCallback_ = cb;}
    void setErrorCallback(const EventCallback& cb) {errorCallback_ = cb;}

    int fd() const {return fd_;}
    int events() const {return events_;}
    void set_revents(int revt) {revents_ = revt;}
    bool isNoneEvent() const {return events_ == kNoneEvent;}

    void enableReading() {events_ |= kReadEvent; update();}
    // void enableWriting() {events_ |= kWriteEvent; update();}
    // void disableWriting() {events_ &= ~kWriteEvent; update();}
    // void disableAll() {events_ = kNoneEvent; update();}

    int index() {return index_;}
    void set_index(int idx) {index_ = idx;}

    EventLoop* ownerLoop() {return loop_;}

private:
    void update();

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    
    EventLoop* loop_;
    const int fd_;  // 负责的文件描述符
    int events_;    // 关心的IO事件
    int revents_;   // 目前活动的事件
    int index_;     // 供 Epoll 使用

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
};