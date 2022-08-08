#include"TcpServer.h"
#include"loger.h"
#include"InetAddress.h"

#include<string.h>


using namespace std::placeholders;

static EventLoop* checkNoNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("NO LOOP ");
    }
    return loop;
}


TcpServer::TcpServer(EventLoop* loop,
        const InetAddress& listenAddr,
        const std::string& nameArg,
        Option option)
    :   loop_(checkNoNull(loop)),
        ipPort_(listenAddr.toIpPort()),
        name_(nameArg),
        acceptor_(new Acceptor(loop, listenAddr, option == kReuserPort)),
        threadpool_(new EventLoopThreadPool(loop_, nameArg)),
        connetionCallback_(),
        messageCallback_(),
        nextConnId_(1)
{
    //当有新用户连接时调用TcpServer::newConnection函数，不做读写，通过轮询算法唤醒subreator进行处理
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this, _1,_2));
}

void TcpServer::start()
{
    if(started++ == 0)//原子类保证只有一个Tcpserver
    {
        threadpool_->start(threadInitCallback_);

        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    threadpool_->setThreadNum(numThreads);
}

TcpServer::~TcpServer()
{
    for(auto& item : connections_)
    {
        TcpConnectionPtr conn(item.second);//使用局部变量掌握资源，出了作用域直接释放
        item.second.reset();
        conn->getloop()->runInLoop(std::bind(&TcpConnection::connectDestroyed,conn));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    EventLoop* ioloop = threadpool_->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s\n",name_.c_str(),connName.c_str(), peerAddr.toIpPort().c_str());

    //通过socketfd获取本地的地址
    sockaddr_in local;
    bzero(&local, sizeof(local));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(local));
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen));
    {
        LOG_ERROR("Tcp::newConnection\n")
    }
    InetAddress localAddr(local);


    //创建tcpconnction
    TcpConnectionPtr conn(new TcpConnection(ioloop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    //下面的回调，用户-》TcpServer-》TcpConnection-》channel-》poller（注册）
    conn->setConnectionCallback(connetionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    

    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    ioloop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO("TcpServer::removerConnctionInLoop [%s] - connection %s\n",name_.c_str(),conn->name().c_str());

    connections_.erase(conn->name());
    EventLoop* ioloop = conn->getloop();
    ioloop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}