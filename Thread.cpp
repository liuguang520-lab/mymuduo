
#include "Thread.h"
#include "CurrentThread.h"

#include<semaphore.h>
using namespace lg;

std::atomic_int32_t Thread::numCreaded_(0);

Thread::Thread(ThreadFunc func, const std::string& name )
    : started_(false),
      joined_(false),
      tid_(0),
      func_(std::move(func)),
      name_(name)
{
    setDefalutName();
}
Thread::~Thread()
{
    if(started_ && !joined_)
    {
        thread_->detach();
    }
}

void Thread::start()
{
    sem_t sem;
    sem_init(&sem, false, 0);
    started_ = true;
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        //获取线程id
        tid_ = lg::CurrentThread::tid();
        sem_post(&sem);
        func_();
    }));
    //必须等待上面的线程创建完成
    sem_wait(&sem);
}
void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefalutName()
{
    int num = ++numCreaded_;
    if(name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}