#pragma once

#include "thread/Thread.h"
#include "EventLoop.h"
#include "../utils/nocopyable.h"

class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();
    void threadFunc();

private:
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;

    EventLoop * loop_;
    bool exiting_;
};