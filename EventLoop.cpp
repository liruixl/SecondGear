#include "EventLoop.h"

#include <sys/poll.h>
#include "Poller.h"

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
:looping_(false),
threadId_(CurrentThread::tid()),
quit_(false)
{
    if(t_loopInThisThread)
    {
        std::cout  << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
        assert(t_loopInThisThread == NULL);
    }
    else{
        t_loopInThisThread = this;
    }

    //move assign
    poller_ = std::unique_ptr<Poller>(new Poller(this));
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = NULL;
}

void EventLoop::assertInLoopThread()
 {
    if(!isInLoopThread())
    {
        std::cout << "EventLoop::abortNotInLoopThread - EventLoop " << this
        << " was created in threadId_ = " << threadId_
        << ", current thread id = " <<  CurrentThread::tid() << std::endl;
        assert(false);
    }
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    while(!quit_)
    {
        activeChannels_.clear();
        activeChannels_ = poller_->poll(10000);

        for(auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
        {
            (*it)->handleEvent();
        }
    }

    printf("EventLoop %p stop looping\n", this);
    looping_ = false;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

void EventLoop::quit()
{
    quit_ = true;
    //wakeup();
}

void EventLoop::updateChannel(ChannelPtr channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

