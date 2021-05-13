#include "codec.h"
#include "net/base/Mutex.h"
#include "net/base/noncopyable.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"

#include <set>
#include <stdio.h>
#include <unistd.h>
#include <string>

class ChatServer : noncopyable{
public:
    ChatServer(EventLoop* loop, const InetAddress& listenAddr)
        :server_(loop, listenAddr, ""),
         codec_(std::bind(&ChatServer::onStringMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
    {
        server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &codec_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void start(){
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn){
        if (conn->connected()){
            connections_.insert(conn);
        }
        else{
            connections_.erase(conn);
        }
    }

    void onStringMessage(const TcpConnectionPtr&, const std::string& message, int64_t receiveTime){
        for(auto it = connections_.begin(); it != connections_.end(); it++){
            codec_.send((*it).get(), message);
        }
    }

    typedef std::set<TcpConnectionPtr> ConnectionList;
    TcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;
};

int main(int argc, char* argv[]){
    if(argc > 1){
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress serverAddr(port);
        ChatServer server(&loop, serverAddr);
        server.start();
        loop.loop();
    }
    else{
        printf("Usage: %s port\n", argv[0]);
    }
}
