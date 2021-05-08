#pragma once
#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"
#include "CurrentThread.h"

class MutexLock : noncopyable{
public:
    MutexLock() : holder_(0){
        pthread_mutex_init(&mutex_, NULL);
    }

    ~MutexLock(){
        pthread_mutex_lock(&mutex_);
        pthread_mutex_destroy(&mutex_);
    }

    bool isLockByThisThread(){
        return holder_ == CurrentThread::tid();
    }

    void lock(){ //仅供 MutexLockGuard 调用
        pthread_mutex_lock(&mutex_);
        holder_ = CurrentThread::tid();
    }

    void unlock(){ //仅供 MutexLockGuard 调用
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
    }

    pthread_mutex_t* getPthreadMutex(){ //仅供 Condition 调用
        return &mutex_;
    }
private:
    pthread_mutex_t mutex_;
    pid_t holder_;
};

class MutexLockGuard : noncopyable{
public:
    explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex){
        mutex_.lock();
    }
    
    ~MutexLockGuard(){
        mutex_.unlock();
    }
private:
    MutexLock& mutex_;
};