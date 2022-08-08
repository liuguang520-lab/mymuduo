#include "EpollPoller.h"
#include "loger.h"
#include "Channel.h"

#include<unistd.h>
#include<errno.h>
#include<string.h>

using namespace lg;

//表示channel未添加到poller中
const int kNew = -1;
//表示channel已经添加到poller中
const int kAdded = 1;
//表示channel被删除了
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop)
    :Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("epoll_creat fatal %d\n", errno);
    }   
}
EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeouMs, ChannelList* activeChannels)
{
    LOG_INFO("fd total count %lu \n", channels_.size());
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                 static_cast<int>(events_.size()), timeouMs);
    int saveErrno = errno;//防止其他线程对errno的更改
    Timestamp now(Timestamp::now());
    if(numEvents > 0)
    {
        //表示有事件发生
        LOG_INFO("%d events happen", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size())
        {
            events_.resize(2*events_.size());//扩容
        }
    }
    else if(numEvents == 0)
    {
        LOG_INFO("noting happen");
    }
    else
    {
        if(saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPOLLPOLLER::POLL");
        }
    }
    return now;
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for(int i = 0; i< numEvents; i++)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revent(events_[i].events);
        activeChannels->push_back(channel);//将监听到的事件放入待处理的数组中
    }
}


void EpollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();

    LOG_INFO("fd=%d, event=%d index=%d \n", channel->fd(), channel->events(), channel->index());
    if(index == kNew || index == kDeleted)
    {
        //表示从来没有进入epoll或者被删除了的节点
        int fd = channel->fd();
        if(index == kNew)
        {
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);//更改channel的index
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        //已经添加到poller中，那么就是删除或者修改事件
        int fd = channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    channels_.erase(fd);//从map中删除
    if(channel->index() == kAdded)
    {
        //如果还处在poller上面就需要调用epoll_ctl进行删除
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(kDeleted);
}

void EpollPoller::update(int operation,  Channel* channel)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.fd = channel->fd();
    event.data.ptr = channel;

    int fd = channel->fd();
    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll del error: %d\n",errno);
        }
        else
        {
            LOG_FATAL("epoll add/mod error: %d\n", errno);
        }
    }
}

