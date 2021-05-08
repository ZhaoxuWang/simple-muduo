#pragma once
#include "base/noncopyable.h"

int createNonblocking();
struct sockaddr_in getLocalAddr(int sockfd);
int getSocketError(int sockfd);

class InetAddress;

class Socket : noncopyable{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();
    int fd() const {return sockfd_;}
    void bindAddress(const InetAddress& localaddr);
    void listen();
    // 返回已经设置了non-blocking and close-on-exec的fd
    int accept(InetAddress* peeraddr);
    void setReuseAddr(bool on);

    void shutdownWrite();

    void setTcpNoDelay(bool on);

private:
    const int sockfd_;
};