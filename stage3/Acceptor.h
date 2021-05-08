#pragma once
#include "Channel.h"
#include "Socket.h"
#include "base/noncopyable.h"

#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable{
public:
    typedef std::function<void (int, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr);
    void setNewConnectionCallback(const NewConnectionCallback& cb) {newConnectionCallback_ = cb;}

    bool listenning() const { return listenning_; }
    void listen();

private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};