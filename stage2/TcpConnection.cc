#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <functional>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name, int sockfd,
                                const InetAddress& localAddr, const InetAddress& peerAddr)
    :loop_(loop),
     name_(name),
     state_(kConnecting),
     socket_(new Socket(sockfd)),
     channel_(new Channel(loop, sockfd)),
     localAddr_(localAddr),
     peerAddr_(peerAddr)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection(){}

void TcpConnection::send(const std::string& message){
    if(state_ == kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(message);
        }
        else{
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message){
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // 如果 output buffer 中没内容，直接写
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0){
        nwrote = write(channel_->fd(), message.data(), message.size());
        if(nwrote < 0){
            nwrote = 0;
            if(errno != EWOULDBLOCK){
                perror("TcpConnection::sendInLoop");
                abort();
            }
        }
        else if(static_cast<size_t>(nwrote) == message.size()){
            if(writeCompleteCallback_){
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
    }
    // 没写完，或 output buffer 中原本就有内容
    if(static_cast<size_t>(nwrote) < message.size()){
        outputBuffer_.append(message.data()+nwrote, message.size()-nwrote);
        if(!channel_->isWriting()){
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown(){
    if (state_ == kConnected){
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop(){
    loop_->assertInLoopThread();
    if(!channel_->isWriting()){
        socket_->shutdownWrite();
    }
}

void TcpConnection::setTcpNoDelay(bool on){
    socket_->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished(){
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);

    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed(){
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead(int64_t receiveTime){
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if(n > 0){
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if(n == 0){
        printf("from TcpConnection::handleRead");
        handleClose();
    }
    else{
        errno = savedErrno;
        handleError();
    }
}

void TcpConnection::handleWrite(){
    loop_->assertInLoopThread();
    if(channel_->isWriting()){
        ssize_t n = write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if(n > 0){
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0){
                channel_->disableWriting();
                if(writeCompleteCallback_){
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(state_ == kDisconnecting){
                    shutdownInLoop();
                }
            }
        }
        else{
            perror("TcpConnection::handleWrite");
            abort();
        }
    }
    else{
        // printf("Connection is down, no more writing\n");
    }
}

void TcpConnection::handleClose(){
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    channel_->disableAll();
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError(){
    int err = getSocketError(channel_->fd());
    perror(strerror(err));
}