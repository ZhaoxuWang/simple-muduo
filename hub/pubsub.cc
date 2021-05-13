#include "pubsub.h"
#include "codec.h"

PubSubClient::PubSubClient(EventLoop* loop, const InetAddress& hubAddr, const std::string& name) : client_(loop, hubAddr, name){
    client_.setConnectionCallback(std::bind(&PubSubClient::onConnection, this, std::placeholders::_1));
    client_.setMessageCallback(
      std::bind(&PubSubClient::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void PubSubClient::start(){
    client_.connect();
}

void PubSubClient::stop(){
    client_.disconnect();
}

bool PubSubClient::connected() const{
    return conn_ && conn_->connected();
}

bool PubSubClient::subscribe(const std::string& topic, const SubscribeCallback& cb){
    std::string message = "sub " + topic + "\r\n";
    subscribeCallback_ = cb;
    return send(message);
}

void PubSubClient::unsubscribe(const std::string& topic){
    std::string message = "unsub " + topic + "\r\n";
    send(message);
}

bool PubSubClient::publish(const std::string& topic, const std::string& content){
    std::string message = "pub " + topic + "\r\n" + content + "\r\n";
    send(message);
}

void PubSubClient::onConnection(const TcpConnectionPtr& conn){
    if(conn->connected()){
        conn_ = conn;
    }
    else{
        conn_.reset();
    }

    if(connectionCallback_){
        connectionCallback_(this);
    }
}

void PubSubClient::onMessage(const TcpConnectionPtr& conn, Buffer* buf, int64_t receiveTime){
    ParseResult result = kSuccess;
    while (result == kSuccess){
        std::string cmd;
        std::string topic;
        std::string content;
        result = parseMessage(buf, &cmd, &topic, &content);
        if(result == kSuccess){
            if(cmd == "pub" && subscribeCallback_){
                subscribeCallback_(topic, content, receiveTime);
            }
        }
        else if(result == kError){
            conn->shutdown();
        }
    }
}

bool PubSubClient::send(const std::string& message){
    bool succeed = false;
    if(conn_ && conn_->connected()){
        conn_->send(message);
        succeed = true;
    }
    return succeed;
}