#pragma once
#include"nocopyable.h"
#include"Callbacks.h"
#include"Buffer.h"
#include"InetAddress.h"
#include"Timestamp.h"

#include<atomic>
#include<string>
#include<memory>
namespace lg
{

class Channel;
class EventLoop;
class Socket;

/*
Tcpserver => Acceptor => 有一个新用户连接就调用accept返回connfd
=> TcpConnection 设置回调 => Channel => Poller => Channel的回调操作
*/

//表示已经建立连接的通信
class TcpConnection : nocopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getloop()const { return loop_; }
    const std::string& name()const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddredss() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const {return state_ == kDisconnected; }


    void send(const std::string& buf);
    void shutdown();

    void connectEstablished();//创建一个新连接的时候调用
    void connectDestroyed();//从tcpserver的map上删除的时候

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb;}

    Buffer* inputBuffer()
    { return &inputBuffer_; }

    Buffer* outputBuffer()
    { return &outputBuffer_; }

private:
    enum StateR{kDisconnected, kConnecting, kConnected, kDisconnecting };

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void setState(StateR s) { state_ = s; }

    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();

    EventLoop* loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;


    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_;
    
    Buffer inputBuffer_;
    Buffer outputBuffer_;

};

}