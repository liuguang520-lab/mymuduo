#pragma once

#include "Timestamp.h"
#include "CurrentThread.h"
#include "nocopyable.h"

#include<vector>
#include<functional>
#include<atomic>
#include<memory>
#include<mutex>
//EventLoop主要包含两个大的类，channel 和poller(epoll)
namespace lg
{

class Poller;
class Channel;

class EventLoop : nocopyable
{
 public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();//开启事件循环
    void quit();

    Timestamp pollReturnTIme() const{ return pollReturnTime_;}

    void runInLoop(Functor cb);//在当前loop中执行cb
    void queueInLoop(Functor cb);//将cb放入队列中

    // 内部使用
    void wakeup();
    void updateChannel(Channel* channel);//在channel中进行调用
    void removeChannel(Channel* Channel);
    bool hasChannel(Channel* Channel);

    //判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread()const { return threadId_ == CurrentThread::tid();}

 private:
    void handleRead(); //wake up
    void doPendingFunctor();//执行回调

    using ChannelList = std::vector<Channel*>;

    //控制循环
    std::atomic_bool loop_;
    std::atomic_bool quit_;

    const pid_t threadId_;//表示创建Eventloop的线程ID

    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;//主要作用是当mainloop获取一个新用户channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    
    std::atomic_bool callingPendingFunctors_;//标识是否有需要执行的回调
    std::vector<Functor> pendingFunctors_;
    std::mutex mutex_;
};


}