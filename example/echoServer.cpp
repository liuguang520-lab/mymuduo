#include<mymuduo/TcpServer.h>
#include<string>
#include<mymuduo/loger.h>

using namespace lg;

class EchoServer
{
public:
    EchoServer(EventLoop* loop,
    const InetAddress& addr, const std::string& name)
    :   server_(loop, addr, name),
        loop_(loop)
    {
        //注册回调函数
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection,this, std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        server_.setThreadNum(2);
    }
    void start()
    {
        server_.start();
    }
private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        if(conn->connected())
        {
            LOG_INFO("connection up : %s", conn->peerAddredss().toIpPort().c_str());
        }
        else
        {
            LOG_INFO("connection down : %s", conn->peerAddredss().toIpPort().c_str());
        }
        
    }
    void onMessage(const TcpConnectionPtr&conn, Buffer* buf, Timestamp time)
    {
        std::string msg = buf->retriveAllAsString();
        conn->send(msg);
        conn->shutdown();
    } 

    EventLoop* loop_;
    TcpServer server_;
};


int main()
{

    EventLoop* loop;
    InetAddress addr(8008);
    EchoServer server(loop, addr, "server::1");
    server.start();
    loop->loop();//主循环开启

    return 0;
}