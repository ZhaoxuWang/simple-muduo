#include "codec.h"
#include "net/base/Mutex.h"
#include "net/base/noncopyable.h"
#include "net/EventLoopThread.h"
#include "net/TcpClient.h"

#include "pubsub.h"

#include <set>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <string>

class Multi_chat : noncopyable{
public:
    Multi_chat(EventLoop* loop, const InetAddress& serverAddr)
        :client_(loop, serverAddr, "")
    {
        client_.setConnectionCallback(std::bind(&Multi_chat::onConnection, this, std::placeholders::_1));
        client_.setMessageCallback(
        std::bind(&Multi_chat::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void connect(){
        client_.connect();
    }

    void disconnect(){
        client_.disconnect();
    }

    void write(std::string& message){
        std::string cmd;
        std::string topic;
        std::string content;
        ParseResult result = parseMessage(message, &cmd, &topic, &content);
        
        if(result == kSuccess){
            // printf("%s#%s#%s", cmd.c_str(), topic.c_str(), content.c_str());
            MutexLockGuard lock(mutex_);
            if(connection_){
                std::string tosend;
                if(cmd == "pub"){
                    tosend = cmd + " " + topic + "\r\n" + content + "\r\n";
                }
                else{
                    tosend = cmd + " " + topic + "\r\n";
                }
                (*connection_.get()).send(tosend);
            }
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

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, int64_t receiveTime){
        std::string cmd;
        std::string topic;
        std::string content;
        ParseResult result = parseMessage(buf, &cmd, &topic, &content);
        printf("%s: %s\n", topic.c_str(), content.c_str());
    }

    TcpClient client_;
    MutexLock mutex_;
    TcpConnectionPtr connection_; // @GUARDED_BY(mutex_)
};

int main(int argc, char* argv[]){
    if (argc > 2){
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);

        Multi_chat client(loopThread.startLoop(), serverAddr);
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