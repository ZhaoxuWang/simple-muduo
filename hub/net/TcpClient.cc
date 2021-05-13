#include "TcpClient.h"
#include "Connector.h"
#include "EventLoop.h"
#include "Socket.h"

#include <functional>
#include <stdio.h>

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn){
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector){
    //connector->
}

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& nameArg)
    :loop_(loop),
     name_(nameArg),
     connector_(new Connector(loop, serverAddr)),
     retry_(false),
     connect_(true),
     nextConnId_(1)
{
    connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient(){
    TcpConnectionPtr conn;
    {
        MutexLockGuard lock(mutex_);
        conn = connection_;
    }
    if(conn){
        CloseCallback cb = std::bind(::removeConnection, loop_, std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
    }
    else{
        connector_->stop();
        loop_->runAfter(1, std::bind(::removeConnector, connector_));
    }
}

void TcpClient::connect(){
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect(){
    connect_ = false;
    {
        MutexLockGuard lock(mutex_);
        if(connection_){
            connection_->shutdown();
        }
    }
}

void TcpClient::stop(){
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd){
    loop_->assertInLoopThread();
    InetAddress peerAddr(getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = buf;
    InetAddress localAddr(getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));

    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn){
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

    if(retry_ && connect_){
        connector_->restart();
    }
}