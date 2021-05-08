#include "InetAddress.h"

#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

InetAddress::InetAddress(uint16_t port){
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port){
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}

std::string InetAddress::toHostPort() const{
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)); 
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + strlen(buf), ":%u", port);
    return buf;
}