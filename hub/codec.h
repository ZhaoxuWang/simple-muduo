#pragma once
#include "net/base/noncopyable.h"
#include "net/Buffer.h"
#include "net/TcpConnection.h"

#include <string>

enum ParseResult
{
  kError,
  kSuccess,
  kContinue,
};

ParseResult parseMessage(Buffer* buf, std::string* cmd, std::string* topic, std::string* content);
ParseResult parseMessage(std::string& buf, std::string* cmd, std::string* topic, std::string* content);

class LengthHeaderCodec : noncopyable{
public:
    typedef std::function<void (const TcpConnectionPtr&, const std::string&message, int64_t)> StringMessageCallback;

    explicit LengthHeaderCodec(const StringMessageCallback& cb) : messageCallback_(cb) { }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, int64_t receiveTime){
        while (buf->readableBytes() >= kHeaderLen){
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data);
            const int32_t len = be32toh(be32);
            if(len > 65536 || len < 0){
                perror("Invalid length");
                conn->shutdown();
                break;
            }
            else if(buf->readableBytes() >= len + kHeaderLen){
                buf->retrieve(kHeaderLen);
                std::string message(buf->peek(), len);
                messageCallback_(conn, message, receiveTime);
                buf->retrieve(len);
            }
            else{
                break;
            }
        }
    }

    void send(TcpConnection* conn, std::string message){
        Buffer buf;
        buf.append(message.data(), message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = htobe32(len);
        buf.prepend(&be32, sizeof(be32));
        conn->send(buf.retrieveAsString());
    }

private:
    StringMessageCallback messageCallback_;
    const static size_t kHeaderLen = sizeof(int32_t);
};
