#pragma once

#include <iostream>
#include <assert.h>

#include "utils/nocopyable.h"
#include "utils/CurrentThread.h"
#include "utils/Thread.h"

class EventLoop : noncopyable
{
public:
    EventLoop();
    ~EventLoop();
    static EventLoop* getEventLoopOfCurrentThread();

    void loop();

    void assertInLoopThread();
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid();}

private:

    //void abortNotInLoopThread();
    bool looping_; /*atomic*/
    const pid_t threadId_;

};
