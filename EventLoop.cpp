#include "EventLoop.h"

#include <sys/poll.h>

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
:looping_(false),
threadId_(CurrentThread::tid())
{
    if(t_loopInThisThread)
    {
        std::cout  << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
    }
    else{
        t_loopInThisThread = this;
    }
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

    std::cout << " sys call poll 5s" << std::endl;
    ::poll(NULL, 0, 5*1000);

    looping_ = false;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}
