#pragma once

#include"nocopyable.h"

#include<functional>
#include<memory>
#include<string>
#include<vector>

namespace lg
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads){ numThreads_ = numThreads;}
    void start(const ThreadInitCallback& cb= ThreadInitCallback());

    EventLoop* getNextLoop();//采用轮询的方式
    std::vector<EventLoop*> getAllLoops();

    bool started() const{ return started_;}
    const std::string& name() const {return name_;}
private:
    
    EventLoop* baseloop_;//每次调用时都会有一个baseloop
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;//下一个loop的下标，采用轮询的机制进行访问
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loop_;
};

}