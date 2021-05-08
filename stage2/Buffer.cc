#include "Buffer.h"

#include <errno.h>
#include <memory.h>
#include <sys/uio.h>

ssize_t Buffer::readFd(int fd, int* savedErrno){
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    // 两块缓冲区，一块在buffer中，一块是栈区新建的extrabuf。
    vec[0].iov_base = begin()+writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // readv用于在一次函数调用中读入多个非连续缓冲区，也叫 scatter read
    const ssize_t n = readv(fd, vec, 2);
    if(n < 0){
        *savedErrno = errno;
    }
    else if(static_cast<size_t>(n) <= writable){
        writerIndex_ += n;
    }
    else{
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}