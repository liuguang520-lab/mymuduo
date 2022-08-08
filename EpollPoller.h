#include<vector>
#include<sys/epoll.h>

#include "Poller.h"


/*
epoll_create
epoll_ctl
epoll_wait
*/

namespace lg
{
class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    //重写父类
    Timestamp poll(int timeouMs, ChannelList* activeChannels)override;//调用epoll_wait

    //在Channel类中通过EventLoop类进行调用
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

    void update(int operation,  Channel* channel);

    using EventList = std::vector<epoll_event>;//初始化16

    int epollfd_;
    EventList events_;//epoll返回的事件
};
}