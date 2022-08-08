#include"TcpConnection.h"
#include"loger.h"
#include"Socket.h"
#include"Channel.h"
#include"EventLoop.h"

#include<errno.h>
#include<string>


using namespace lg;
using namespace std::placeholders;

static EventLoop* checkNoNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("NO LOOP ");
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop,
            const std::string& name,
            int sockfd,
            const InetAddress& localAddr,
            const InetAddress& peerAddr)
    :   loop_(checkNoNull(loop)),
        name_(name),
        state_(kConnecting),
        socket_(new Socket(sockfd)),
        channel_(new Channel(loop, sockfd)),
        localAddr_(localAddr),
        peerAddr_(localAddr),
        highWaterMark_(64*1024*1024)
{
    //下面给channel设置的回调函数，当poller监听到事件发生时会调用下面的函数
        channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, _1));

    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));

    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
    LOG_INFO("TcpConnection::ctor[%s] at fd=%p\n", name_.c_str(), socket_.get());
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnect::dtor[%s] at fd=%p\n", name_.c_str(), socket_.get());

}



void TcpConnection::handleRead(Timestamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFD(channel_->fd(), &saveErrno);
    if(n > 0)
    {
        //share_from_this 表示获取该类的智能指针，也就是TcpConnection的指针
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if( n== 0)
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead\n");
        handleError();
    }
}
void TcpConnection::handleWrite()
{
    if(channel_->isWriting())
    {
        int savaErrno = 0;
        ssize_t n = outputBuffer_.writeFD(channel_->fd(), &savaErrno);
        if(n > 0)
        {
            outputBuffer_.retrive(n);
            if(outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(state_ == kDisconnected)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite\n")
        } 
    }
    else
    {
        LOG_ERROR("Connection fd =%d is down, no more writing\n",channel_->fd());
    }
}
void TcpConnection::handleClose()
{
    LOG_INFO("fd = %d state = %d",channel_->fd(), (int)state_);
    setState(kDisconnected);
    channel_->disableAll();//断开连接
    
    TcpConnectionPtr conn(shared_from_this());
    connectionCallback_(conn);
    closeCallback_(conn);
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t oplen = static_cast<socklen_t>(sizeof(optval));
    int err = 0;
    if(::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &oplen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n", name_.c_str(), err);

}

void TcpConnection::send(const std::string& buf)
{   
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str(), buf.size());
        }
        else
        {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,this, buf.c_str(), buf.size()));
        }
    }
}
//发送数据， 应用写的快，内核发送的慢，需要将待发送的数据写入缓冲区，并且设置水位
void TcpConnection::sendInLoop(const void* message, size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool falutError = false;
    if( state_ == kDisconnected)
    {
        LOG_ERROR("disconnected, give up writing");
        return ;
    }
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), message, len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            if(remaining == 0&& writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else 
        {
            nwrote = 0;
            if(errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop");
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    falutError = true;
                }
            }
        }
    }
    //表示数据没有一次性发送完毕，需要放入缓冲区进行第二次发送，并且需要注册读事件回调

    if(!falutError && remaining > 0)
    {
        size_t oldlen = outputBuffer_.readableBytes();
        if(oldlen + remaining >= highWaterMark_&& oldlen < highWaterMark_
        && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldlen+remaining));
        }
        outputBuffer_.append((char*)message, remaining);
        if(!channel_->isWriting())
        {
            channel_->enableWriting();//一定要注册读事件
        }
    }
}
void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}
void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);

        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}


void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}
void TcpConnection::connectDestroyed()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();//从poller中删除
}
