#pragma once

#include<vector>
#include<unordered_map>

#include"nocopyable.h"
#include"Timestamp.h"
#include"EventLoop.h"
//poller是一个抽象类，在muduo中拥有eppoll和pill两个派生类，poller只是一些通用接口，具体实现需要看派生类
namespace lg
{

class Channel;

class Poller : nocopyable
{
public:

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    using ChannelList = std::vector<Channel*>;

    //使用统一的接口对于eppol和poll
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    //更改IO事件
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    //检查channel是否处在poller中
    virtual bool hasChannel(Channel* channel)const;

    //获取默认的poller实现，在基类中不会去引用派生内的头文件，所以该函数会在公共的文件中实现
    static Poller* newDefalutPoller(EventLoop* loop);
protected:
    // key表示的时socketfd
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerloop_;
};

}