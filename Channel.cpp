
#include "Channel.h"
#include "EventLoop.h"
#include <sys/poll.h>


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
:loop_(loop),
fd_(fd),
events_(0),
revents_(0),
index_(-1)
{ }

//Channel.h未包含EventLoop.h
//必须定义在这
//when update?
void Channel::update()
{
    loop_->updateChannel(shared_from_this());
}


void Channel::handleEvent()
{
    if(revents_ & POLLNVAL)
    {
        printf("Channel::handleEvent() POLLNVAL.\n");
    }
    if(revents_ & (POLLERR | POLLNVAL))
    {
        if(errorCallback) errorCallback();
    }
    if(revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if(readCallback) readCallback();
    }
    if(revents_ & POLLOUT)
    {
        if(writeCallback) writeCallback();
    }
}