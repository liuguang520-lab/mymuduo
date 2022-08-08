#include "Poller.h"
#include "Channel.h"

using namespace lg;

Poller::Poller(EventLoop* loop)
    : ownerloop_(loop)
{
}

bool Poller::hasChannel(Channel* channel) const
{
    ChannelMap::const_iterator cit = channels_.find(channel->fd());
    return cit != channels_.end() && cit->second == channel;
}