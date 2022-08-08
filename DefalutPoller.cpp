#include "Poller.h"
#include"EpollPoller.h"
#include<stdlib.h>
using namespace lg;

Poller* Poller::newDefalutPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        return nullptr;
    }
    else
    {
        return new EpollPoller(loop);
    }
}