#pragma once

#include"nocopyable.h"
#include"Acceptor.h"
#include"EventLoop.h"
#include"EventLoopThreadPool.h"
#include"Callbacks.h"
#include"TcpConnection.h"

#include<string>
#include<functional>
#include<memory>
#include<atomic>
#include<unordered_map>

using namespace lg;

class TcpServer : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option
    {
        kNoReuserPort,
        kReuserPort,
    };

    TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const std::string& nameArg,
            Option option = kNoReuserPort);
    ~TcpServer();

    void setThreadNum(int numThreads);//设置subreactor的个数
    void setThreadInitCallback(const ThreadInitCallback& cb)
    {
        threadInitCallback_ = cb;
    }

    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connetionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    {messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }
private:

    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);


    using ConnectMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;//mainloop, acceptor 中的loop
    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadpool_;
    ConnectionCallback connetionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    std::atomic_int started;

    int nextConnId_;
    ConnectMap connections_;
};
// namespace lg

