#include "codec.h"
#include "net/base/Mutex.h"
#include "net/base/noncopyable.h"
#include "net/EventLoopThread.h"
#include "net/TcpClient.h"

#include <set>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>

class ChatClient : noncopyable{
public:
    ChatClient(EventLoop* loop, const InetAddress& serverAddr)
        :client_(loop, serverAddr, ""),
         codec_(std::bind(&ChatClient::onStringMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
    {
        client_.setConnectionCallback(std::bind(&ChatClient::onConnection, this, std::placeholders::_1));
        client_.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &codec_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void connect(){
        client_.connect();
    }

    void disconnect(){
        client_.disconnect();
    }

    void write(const std::string& message){
        MutexLockGuard lock(mutex_);
        if(connection_){
            codec_.send(connection_.get(), message);
        }
    }

private:
    void onConnection(const TcpConnectionPtr& conn){
        MutexLockGuard lock(mutex_);
        if (conn->connected()){
            connection_ = conn;
        }
        else{
            connection_.reset();
        }
    }

    void onStringMessage(const TcpConnectionPtr&, const std::string& message, int64_t){
        printf("<<< %s\n", message.c_str());
    }

    TcpClient client_;
    LengthHeaderCodec codec_;
    MutexLock mutex_;
    TcpConnectionPtr connection_; // @GUARDED_BY(mutex_)
};

int main(int argc, char* argv[]){
    if (argc > 2){
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);

        ChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();
        std::string line;
        while (std::getline(std::cin, line)){
            client.write(line);
        }
        client.disconnect();
    }
    else{
        printf("Usage: %s host_ip port\n", argv[0]);
    }
}
