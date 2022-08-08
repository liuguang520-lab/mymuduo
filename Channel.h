#pragma once


//该类主要封装了socketfd 和感兴趣的时间EPPOLLIN和EPILLOUT
//还包括了poller返回的事件


#include"Timestamp.h"
#include"nocopyable.h"
#include<functional>
#include<memory>

namespace lg
{

class EventLoop;

class Channel : nocopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>; 

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);//fd 得到poller的通知，处理事件

    //设置回调函数
    void setReadCallback(ReadEventCallback cb)
    {readCallback_ = std::move(cb);}
    void setWriteCallback(EventCallback cb)
    {writeCallback_ = std::move(cb);}
    void setCloseCallback(EventCallback cb)
    {closeCallback_ = std::move(cb);}
    void setErrorCallback(EventCallback cb)
    {errorCallback_ = std::move(cb);}

    void tie(const std::shared_ptr<void>&);

    int fd()const {return fd_;}
    int events()const {return events_;}
    void set_revent(int revt){revents_ = revt;}//这个是poler使用的函数，监听到事件返回
    bool isNoneEvent()const {return events_ == kNodeEvent;}

    //poller的监听事件的变化
    void enableReading(){events_ != kReadEvent; update();}
    void disableReading(){events_ &= ~kReadEvent; update();}
    void enableWriting(){events_ |= kWriteEvent; update();}
    void disableWriting(){events_ &= ~kWriteEvent; update();}
    void disableAll(){events_ = kNodeEvent; update();}

    //在poller当中会调用,和epollpoller中的knew，kadded，kdeled相关联
    int index(){return index_;}
    void set_index(int index){index_ = index;}
    
    //判断读写事件
    bool isWriting()const {return events_ & kWriteEvent;}
    bool isReading()const {return events_ & kReadEvent;}

    void remove();
private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    //说明处理的事件
    static const int kNodeEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;//返回的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    // 因为channe里面有返回的具体事件，所以有该对象自己回调具体的函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};

}