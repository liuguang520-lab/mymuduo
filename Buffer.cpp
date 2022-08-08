#include"Buffer.h"


#include<sys/uio.h>
#include<unistd.h>
using namespace lg;

ssize_t Buffer::readFD(int fd, int* saveErrno)
{
    //通过使用栈内存来帮助我们读取数据，从而是我们的vector分配的内存更加合理
    char extrabuf[65536];//64k
    iovec vec[2];
    const size_t writable = writeableBytes();
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf) ? 2 : 1);

    const ssize_t n = ::readv(fd, vec, iovcnt);
    if(n < 0)
    {
        *saveErrno = errno;
    }
    else if(n <= writable)//读取的数据vector能够包含的时候不需要额外的内存
    {
        writeIndex_ += n;
    }
    else
    {
        writeIndex_ = buffer_.size();
        append(extrabuf, n- writable);
    }
    return n;
}


ssize_t Buffer::writeFD(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if(n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}