#include "codec.h"

ParseResult parseMessage(Buffer* buf, std::string* cmd, std::string* topic, std::string* content){
    ParseResult result = kError;
    const char* crlf = buf->findCRLF();
    if(crlf){
        const char* space = std::find(buf->peek(), crlf, ' ');
        if(space != crlf){
            cmd->assign(buf->peek(), space);
            topic->assign(space+1, crlf);
            if(*cmd == "pub"){
                const char* start = crlf + 2;
                crlf = buf->findCRLF(start);
                if(crlf){
                    content->assign(start, crlf);
                    buf->retrieveUntil(crlf+2);
                    result = kSuccess;
                }
                else{
                    result = kContinue;
                }
            }
            else{
                buf->retrieveUntil(crlf+2);
                result = kSuccess;
            }
        }
        else{
            result = kError;
        }
    }
    else{
        result = kContinue;
    }
    return result;
}

ParseResult parseMessage(std::string& msg, std::string* cmd, std::string* topic, std::string* content){
    ParseResult result = kError;
    int start = 0;
    if(msg[0] == '#'){
        start = 1;
        size_t pos = msg.find(' ', start);
        if(pos != std::string::npos){
            (*cmd) = msg.substr(start, pos - start);
            start = pos + 1;
            (*topic) = msg.substr(start, msg.size() - start);
            result = kSuccess;
        }
    }
    else{
        (*cmd) = "pub";
        size_t pos = msg.find(' ', start);
        if(pos != std::string::npos){
            (*topic) = msg.substr(start, pos - start);
            start = pos + 1;
            (*content) = msg.substr(start, msg.size() - start);
            result = kSuccess;
        }
    }
    return result;
}

