#pragma once

#include "nocopyable.h"

namespace lg
{
class InetAddress;

class Socket : nocopyable
{
public:
    explicit Socket(int sockfd) :sockfd_(sockfd)
    {}
    ~Socket();

    int fd()const { return sockfd_;}

    void bindAddress(const InetAddress& localaddr);

    void listen();

    int accept(InetAddress* peeraddr); //返回的是连接的sockfd

    void shutdownWrite();

    void setTcpNoDelay(bool on);//调用setsockopt
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
private:
    const int sockfd_;
};

}