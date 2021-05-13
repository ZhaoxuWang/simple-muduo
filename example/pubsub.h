#pragma once

#include "net/TcpClient.h"
#include "net/base/noncopyable.h"

class PubSubClient : noncopyable{
public:
    typedef std::function<void (PubSubClient*)> ConnectionCallback;
    typedef std::function<void (const std::string& topic, const std::string& content, int64_t Timestamp)> SubscribeCallback;
    PubSubClient(EventLoop* loop, const InetAddress& hubAddr, const std::string& name);
    void start();
    void stop();
    bool connected() const;

    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    bool subscribe(const std::string& topic, const SubscribeCallback& cb);
    void unsubscribe(const std::string& topic);
    bool publish(const std::string& topic, const std::string& content);

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, int64_t receiveTime);
    bool send(const std::string& message);

    TcpClient client_;
    TcpConnectionPtr conn_;
    ConnectionCallback connectionCallback_;
    SubscribeCallback subscribeCallback_;
};