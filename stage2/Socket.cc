#include "Socket.h"
#include "InetAddress.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>


int createNonblocking(){
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0){
        perror("createNonblocking");
        abort();
    }
    return sockfd;
}

struct sockaddr_in getLocalAddr(int sockfd){
    struct sockaddr_in localaddr;
    bzero(&localaddr, sizeof localaddr);
    socklen_t addrlen = sizeof(localaddr);
    if(getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&localaddr), &addrlen) < 0){
        perror("getLocalAddr");
    }
    return localaddr;
}

int getSocketError(int sockfd){
    int optval;
    socklen_t optlen = sizeof optval;
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0){
        return errno;
    }
    else{
        return optval;
    }
}

void setNonBlockAndCloseOnExec(int sockfd){
    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = fcntl(sockfd, F_SETFL, flags);

    flags = fcntl(sockfd, F_GETFL, 0);
    flags |= FD_CLOEXEC;
    ret = fcntl(sockfd, F_SETFL, flags);
}

Socket::~Socket(){
    close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr){
    struct sockaddr_in addr2bind = addr.getSockAddrInet();
    int ret = bind(sockfd_, reinterpret_cast<struct sockaddr*>(&addr2bind), sizeof(addr2bind) );
    if(ret < 0){
        perror("Socket::bindAddress");
    }
}

void Socket::listen(){
    int ret = ::listen(sockfd_, SOMAXCONN);
    if(ret < 0){
        perror("Socket::listen");
    }
}

int Socket::accept(InetAddress* peeraddr){
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);

    socklen_t addrlen = sizeof(addr);
    int connfd = ::accept(sockfd_, reinterpret_cast<struct sockaddr*>(&addr), &addrlen);

    if(connfd >= 0){
        setNonBlockAndCloseOnExec(connfd);
        peeraddr->setSockAddrInet(addr);
    }
    else{
        int savedErrno = errno;
        perror("Socket::accept\n");
        if(savedErrno == EAGAIN || savedErrno == EINTR || savedErrno == ECONNABORTED){
            printf("later\n");
        }
        else{
            abort();
        }
    }
    
    return connfd;
}

void Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::shutdownWrite(){
    if(shutdown(sockfd_, SHUT_WR) < 0){
        perror("Socket::shutdownWrite");
        abort();
    }
}

void Socket::setTcpNoDelay(bool on){
    int optval = on ? 1 : 0;
    setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}