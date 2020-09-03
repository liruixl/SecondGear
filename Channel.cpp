
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
index_(-1),
eventHandling_(false)
{ }


Channel::~Channel()
{
    assert(!eventHandling_);
}


//Channel.h未包含EventLoop.h
//必须定义在这
//when update?
void Channel::update()
{
    loop_->updateChannel(shared_from_this());
}


void Channel::handleEvent(Timestamp receiveTime)
{
    eventHandling_ = true;

    if(revents_ & POLLNVAL)
    {
        printf("Channel::handleEvent() POLLNVAL.\n");
    }

    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        std::cout << "Channel::handle_event() POLLHUP" << std::endl;
        if (closeCallback_) closeCallback_();
    }

    if(revents_ & (POLLERR | POLLNVAL))
    {
        if(errorCallback) errorCallback();
    }
    if(revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if(readCallback) readCallback(receiveTime);
    }
    if(revents_ & POLLOUT)
    {
        if(writeCallback) writeCallback();
    }

    eventHandling_ = false;
}