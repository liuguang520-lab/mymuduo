#pragma once

#include<functional>

#include"nocopyable.h"
#include"Socket.h"
#include"Channel.h"

namespace lg
{

class EventLoop;
class InetAddress; 

class Acceptor : nocopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }
    void listen();
    bool listening()const { return listening_; }
private:
    void handleRead();

    EventLoop* loop_;//acceptor 使用的是用户定义的哪个baseloop，处在mainreactor中

    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};

}