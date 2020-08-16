#include "EventLoopThread.h"

#include <functional>

EventLoopThread::EventLoopThread()
:thread_( [&](){this->threadFunc();} , "EventLoopThread"),
mutex_(),
cond_(mutex_),
loop_(nullptr),
exiting_(false)
{
    
}

EventLoopThread::~EventLoopThread()
{   
    exiting_ = true;
    loop_->quit();
    thread_.join();
}

EventLoop* EventLoopThread::startLoop()
{   
    assert(thread_.isStarted() == false);
    thread_.start();

    {
        MutexGuard lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait();
        }
    }

    return loop_;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;

    {
        MutexGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }
    
    loop.loop();
}