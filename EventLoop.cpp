#include"EventLoop.h"
#include"loger.h"
#include"Poller.h"
#include"Channel.h"

#include<sys/eventfd.h>

using namespace lg;

__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;

int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        LOG_FATAL("Failed in eventfd");
    }
    return evtfd;//通知subreator有连接
}

/*
                mainreator
        通过wakeupfd_进行通信,而不是通过生产者消费者的队列进行通信
    subreator1  subreator2 subreator3
*/



EventLoop::EventLoop()
    : loop_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefalutPoller(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_DEBUG("EventLoop created %p in thread %d", this, threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exits in this thread %d", t_loopInThisThread, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }
    //设置wakefd的事件以及执行的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop reads %ld bytes instead of 8", n);
    }
}

void EventLoop::loop()
{
    loop_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping\n", this);

    while(!quit_)
    {
        activeChannels_.clear();
        //poller会将监听到的事件返回给eventloop
        //poller会监听两种fd，一类是client的fd，一类是调用eventfd产生的wakeupfd_
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);//监听fd是否有对应的事件发生
        for(Channel* channel : activeChannels_)
        {
            channel->handleEvent(pollReturnTime_);
        }
        doPendingFunctor();//执行回调
    }

    LOG_INFO("EventLoop %p stop looping\n",this);
    loop_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        //当不是在自己的线程执行quit时需要唤醒该loop对应的线程
        wakeup();
    }
}

void EventLoop::wakeup()//通过wakeupfd_唤醒subreactor
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::wakeup() writes %ld bytes insted of 8", n);
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }

    //callingPedingFuntors为true时表示该loop正在执行回调，为了防止回到pooler——》pool阻塞，再次唤醒（写入数据）
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}


//全是调用poller对channel进行处理
void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* Channel)
{
    poller_->removeChannel(Channel);
}

bool EventLoop::hasChannel(Channel* Channel)
{
    return poller_->hasChannel(Channel);
}

void EventLoop::doPendingFunctor()
{
    std::vector<Functor> functors;//定义局部变量，将mainreator中的回调放入自己的loop中
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }


    for(const Functor& functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;

}