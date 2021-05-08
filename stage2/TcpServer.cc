#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Callbacks.h"

#include <functional>
#include <stdio.h>
#include <assert.h>

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
    :loop_(loop),
     name_(listenAddr.toHostPort()),
     acceptor_(new Acceptor(loop, listenAddr)),
     started_(false),
     nextConnId_(1)
{   
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer(){ }

void TcpServer::start(){
    if (!started_){
        started_ = true;
    }

    if(!acceptor_->listenning()){
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr){
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    InetAddress localAddr(getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn){
    loop_->assertInLoopThread();
    size_t n = connections_.erase(conn->name());
    assert(n == 1); (void)n;

    // 这里需要使用queueInLoop而不能runInLoop
    // 因为我们能到这一步，是因为channel的closeCallback_()正在被调用
    // 如果直接在这里connectDestroyed TcpConnection，那就会尝试对TcpConnection进行析构
    // 而TcpConnection中的channel成员却还在执行成员函数。
    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}