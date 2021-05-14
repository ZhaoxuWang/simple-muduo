#pragma once
#include "../net/TcpServer.h"
#include "../net/base/noncopyable.h"

#include <experimental/any>

class HttpRequest;
class HttpResponse;

class HttpServer : noncopyable{
public:
    typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;

    HttpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name);

    EventLoop* getLoop() const { return server_.getLoop(); }

    void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

    void start();

private:
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, int64_t receiveTime);
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

    TcpServer server_;
    HttpCallback httpCallback_;
};