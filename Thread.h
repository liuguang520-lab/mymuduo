#pragma once

#include "nocopyable.h"

#include<memory>
#include<thread>
#include<unistd.h>
#include<functional>
#include<string>
#include<atomic>

namespace lg
{

//该类封装了一个线程作为EventLoopThread的底层Thread->EventLoopThread->EventLoopThreadPool
class Thread : nocopyable
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();

    bool started()const{ return started_; }
    pid_t tid() { return tid_; }
    const std::string& name()const { return name_; }

    static int32_t numCreadted() { return numCreaded_; }
private:

    void setDefalutName();

    bool started_;
    bool joined_;

    std::shared_ptr<std::thread> thread_;//通过指针来创建线程，配合着lamda表达式
    pid_t tid_;
    ThreadFunc func_;//线程的开启函数
    std::string name_;

    static std::atomic_int32_t numCreaded_;
};

}