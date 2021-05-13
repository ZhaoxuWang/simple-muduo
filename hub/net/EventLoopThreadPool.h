#pragma once
#include "base/Condition.h"
#include "base/Mutex.h"
#include "base/Thread.h"
#include "base/noncopyable.h"

#include <memory>
#include <vector>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable{
public:
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start();
    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<EventLoopThread*> threads_;
    std::vector<EventLoop*> loops_;
};