#pragma once

#include "nocopyable.h"
#include "Thread.h"

#include<functional>
#include<mutex>
#include<condition_variable>
namespace lg
{

class EventLoop;

class EventLoopThread : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    //默认的回调是什么都不做
    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startloop();

private:
    void threadFunc();//传给thread的回调函数

    EventLoop* loop_;
    std::mutex mutex_;
    bool exiting_;
    Thread thread_;
    std::condition_variable condi_;
    ThreadInitCallback callback_;
};

}
