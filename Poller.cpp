
#include "Poller.h"

#include <sys/poll.h>
#include <assert.h>

Poller::Poller(EventLoop* loop)
    :ownerloop_(loop) { }

Poller::~Poller() { }

ChannelPtrVec Poller::poll(int timeoutMs)
{
    //pollfds_ shouldn't change when traversed
    printf("sys call poll... %ds \n", timeoutMs/1000);
    int numEvents = ::poll(pollfds_.data(), pollfds_.size(), timeoutMs);
    
    if(numEvents < 0) { perror("poll error"); }

    if(numEvents > 0)
    {
        return getActiveChannels(numEvents);
    }

    return {};
}

ChannelPtrVec Poller::getActiveChannels(int numEvents)
{
    ChannelPtrVec activeChannels;

    for(auto pfd = pollfds_.cbegin(); pfd != pollfds_.cend() && numEvents > 0; ++pfd)
    {
        if(pfd->revents > 0) //与epoll_wait区别
        {
            --numEvents;
            auto iter = channels_.find(pfd->fd);
            assert(iter != channels_.end());

            ChannelPtr ch = iter->second;

            ch->set_revents(pfd->revents);
            activeChannels.push_back(ch);
        }
    }

    return activeChannels;
}

void Poller::updateChannel(ChannelPtr channel)
{
    assertInLoopThread();
    //LOG

    if(channel->index() < 0)
    { //a new one
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = channel->events();
        pfd.revents = 0;
        pollfds_.push_back(pfd);

        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else
    {
        //existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);

        int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];

        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);

        pfd.events =  static_cast<short>(channel->events());
        pfd.revents = 0;
        
        if(channel->isNoneEvent())
        {
            //ignore this fd
            //pfd.fd = -1;
            pfd.fd = -channel->fd()-1;
        }
    }
    
}

void Poller::removeChannel(ChannelPtr channel)
{
    assertInLoopThread();
    std::cout << "fd = " << channel->fd() << std::endl;
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd = pollfds_[idx]; (void)pfd;
    assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
    size_t n = channels_.erase(channel->fd());
    assert(n == 1); (void)n;
    if (idx == static_cast<int>(pollfds_.size()-1)) {
        pollfds_.pop_back();
    } else {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);
        if (channelAtEnd < 0) {
            channelAtEnd = -channelAtEnd-1;
        }
        channels_[channelAtEnd]->set_index(idx);
        pollfds_.pop_back();
    }
}
