#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <functional>
#include <errno.h>
#include <assert.h>

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    :loop_(loop),
     serverAddr_(serverAddr),
     connect_(false),
     state_(kDisconnected),
     retryDelayMs_(kInitRetryDelayMs) { }

Connector::~Connector(){
    loop_->cancel(timerId_);
    assert(!channel_);
}

void Connector::start(){
    connect_ = true;
    loop_->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop(){
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if(connect_){
        connect();
    }
}

void Connector::connect(){
    int sockfd = createNonblocking();
    sockaddr_in addr = serverAddr_.getSockAddrInet();
    int ret = ::connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof addr);
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            close(sockfd);
            abort();
            break;

        default:
            close(sockfd);
            abort();
            // connectErrorCallback_();
            break;
  }
}

void Connector::restart(){
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::stop(){
    connect_ = false;
    loop_->cancel(timerId_);
}

void Connector::connecting(int sockfd){
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(std::bind(&Connector::handleWrite, this));
    channel_->setErrorCallback(std::bind(&Connector::handleError, this));

    channel_->enableWriting();
}

int Connector::removeAndResetChannel(){
    channel_->disableAll();
    loop_->removeChannel(channel_.get());
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockfd;
}

void Connector::resetChannel(){
    channel_.reset();
}

void Connector::handleWrite(){
    if (state_ == kConnecting){
        int sockfd = removeAndResetChannel();
        int err = getSocketError(sockfd);
        if(err){
            retry(sockfd);
        }
        else if(isSelfConnect(sockfd)){
            retry(sockfd);
        }
        else{
            setState(kConnected);
            if(connect_){
                newConnectionCallback_(sockfd);
            }
            else{
                close(sockfd);
            }
        }
    }
    else{
        assert(state_ == kDisconnected);
    }
}

void Connector::handleError(){
    assert(state_ == kConnecting);

    int sockfd = removeAndResetChannel();
    int err = getSocketError(sockfd);
    retry(sockfd);
}

void Connector::retry(int sockfd){
    close(sockfd);
    setState(kDisconnected);

    if(connect_){
        timerId_ = loop_->runAfter(retryDelayMs_/1000.0, std::bind(&Connector::startInLoop, this));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else{
        // printf("do not connect\n");
    }
}