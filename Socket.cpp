#include"Socket.h"
#include"InetAddress.h"
#include"loger.h"


#include<unistd.h>
#include<sys/socket.h>
#include<netinet/tcp.h>
#include<string.h>
using namespace lg;
    
Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& localaddr)
{
    int ret = ::bind(sockfd_,(sockaddr*)localaddr.getSockAddr(),sizeof(sockaddr_in));
    if(ret < 0)
    {
        LOG_FATAL("sockets::bindAddress");
    }
}

void Socket::listen()
{
    if(0 != ::listen(sockfd_, 1024))
    {
        LOG_FATAL("sockets::listen");
    }
}

int Socket::accept(InetAddress* peeraddr) //返回的是连接的sockfd
{
    socklen_t len;
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    int confd = ::connect(sockfd_, (sockaddr*)&addr, len);
    if(confd >= 0)
    {
        peeraddr->setSockAddr(addr);
    }
    return confd;
}
void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_, SHUT_WR) < 0)
    {
        LOG_ERROR("sockets::shutdownWrite");
    }
}

void Socket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
}
void Socket::setReuseAddr(bool on)
{
      int optval = on ? 1 : 0;
        ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
}
void Socket::setReusePort(bool on)
{
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
}
void Socket::setKeepAlive(bool on)
{
      int optval = on ? 1 : 0;
        ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
}