#include"EventLoopThread.h"
#include"EventLoop.h"

using namespace lg;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                const std::string& name)
    :   loop_(nullptr),
        exiting_(false),
        thread_(std::bind(&EventLoopThread::threadFunc, this), name),
        mutex_(),
        condi_(),
        callback_(cb)
{
}
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startloop()
{
    thread_.start();//开启新线程，线程中会调用threadFunc函数（下面的函数）

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            condi_.wait(lock);//主线程会在这里等着新的线程创建完成然后执行，并且吧eventloop返回
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;//在新的线程中创建一个Eventloop，对应与one loop per thread
    if(callback_)
    {
        callback_(&loop);//执行回调，参数是loop
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        condi_.notify_one();
    }

    loop.loop();

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}