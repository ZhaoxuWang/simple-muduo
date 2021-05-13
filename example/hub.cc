#include "codec.h"
#include "net/EventLoop.h"
#include "net/TcpServer.h"
#include "net/utils.h"

#include <map>
#include <set>
#include <stdio.h>
#include <functional>

typedef std::set<std::string> ConnectionSubscription;

class Topic{
public:
    Topic(const std::string& topic) : topic_(topic){}

    void add(const TcpConnectionPtr& conn){
        audiences_.insert(conn);
        if(lastPubTime_ > 0){
             conn->send(makeMessage());
        }
    }

    void remove(const TcpConnectionPtr& conn){
        audiences_.erase(conn);
    }

    void publish(const std::string& content, int64_t time){
        content_ = content;
        lastPubTime_ = time;
        std::string message = makeMessage();
        for (std::set<TcpConnectionPtr>::iterator it = audiences_.begin(); it != audiences_.end(); ++it){
            (*it)->send(message);
        }
    }

private:
    std::string makeMessage(){
        return "pub " + topic_ + "\r\n" + content_ + "\r\n";
    }

    std::string topic_;
    std::string content_;
    int64_t lastPubTime_;
    std::set<TcpConnectionPtr> audiences_;
};

class PubSubServer : noncopyable{
public:
    PubSubServer(EventLoop* loop, const InetAddress& listenAddr) : loop_(loop), server_(loop, listenAddr, "PubSubServer"){
        server_.setConnectionCallback( std::bind(&PubSubServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
        std::bind(&PubSubServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    void start(){
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn){
        if(conn->connected()){
            ConnectionSubscription* connSub = conn->getMutableContext();
            connSub->clear();
        }
        else{
            const ConnectionSubscription& connSub = conn->getContext();
            for(ConnectionSubscription::const_iterator it = connSub.begin(); it != connSub.end();){
                doUnsubscribe(conn, *it++);
            }
        }
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, int64_t receiveTime){
        ParseResult result = kSuccess;
        while(result == kSuccess){
            std::string cmd;
            std::string topic;
            std::string content;
            result = parseMessage(buf, &cmd, &topic, &content);
            if(result == kSuccess){
                printf("%s#%s#%s", cmd.c_str(), topic.c_str(), content.c_str());
                if(cmd =="pub"){
                    doPublish(conn->name(), topic, content, receiveTime);
                }
                else if(cmd == "sub"){
                    doSubscribe(conn, topic);
                }
                else if(cmd == "unsub"){
                    doUnsubscribe(conn, topic);
                }
                else{
                    conn->shutdown();
                    result = kError;
                }
            }
            else if(result == kError){
                conn->shutdown();
            }
        }
    }

    void doSubscribe(const TcpConnectionPtr& conn, const std::string& topic){
        ConnectionSubscription* connSub = conn->getMutableContext();
        connSub->insert(topic);
        getTopic(topic).add(conn);
    }

    void doUnsubscribe(const TcpConnectionPtr& conn, const std::string& topic){
        getTopic(topic).remove(conn);
        ConnectionSubscription* connSub = conn->getMutableContext();
        connSub->erase(topic);
    }

    void doPublish(const std::string& source, const std::string& topic, const std::string& content, int64_t time){
        getTopic(topic).publish(content, time);
    }

    Topic& getTopic(const std::string& topic){
        std::map<std::string, Topic>::iterator it = topics_.find(topic);
        if(it == topics_.end()){
            it = topics_.insert(make_pair(topic, Topic(topic))).first;
        }
        return it->second;
    }

    EventLoop* loop_;
    TcpServer server_;
    std::map<std::string, Topic> topics_;
};

int main(int argc, char* argv[]){
    if(argc > 1){
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        EventLoop loop;
        PubSubServer server(&loop, InetAddress(port));
        server.start();
        loop.loop();
    }
    else{
        printf("Usage: %s pubsub_port [inspect_port]\n", argv[0]);
    }
}