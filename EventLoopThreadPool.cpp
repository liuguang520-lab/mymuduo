
#include"EventLoopThreadPool.h"
#include"EventLoopThread.h"

using namespace lg;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop,
                                         const std::string& nameArg)
    :   baseloop_(baseloop),
        name_(nameArg),
        started_(false),
        numThreads_(0),
        next_(0)
{
}
EventLoopThreadPool::~EventLoopThreadPool()
{
    //析构函数没必要，EventLoop对象是在栈上产生的
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    started_ = true;
    for(int i = 0; i< numThreads_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loop_.push_back(t->startloop());
    }
    if(numThreads_ == 0 && cb)//没有设置线程数量，默认只有主线程
    {
        cb(baseloop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()//采用轮询的方式
{
    EventLoop* loop = baseloop_;
    if(!loop_.empty())
    {
        loop = loop_[next_];
        ++next_;
        if(next_ >= loop_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}
std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if(loop_.empty())
    {
        return std::vector<EventLoop*>(1, baseloop_);
    }
    else
    {
        return loop_;
    }
}