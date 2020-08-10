#pragma once

#include <iostream>
#include <memory>
#include <assert.h>

#include "utils/nocopyable.h"
#include "utils/CurrentThread.h"
#include "utils/Thread.h"

#include "Channel.h"

class Poller;

class EventLoop : noncopyable
{
public:
    EventLoop();
    ~EventLoop();
    static EventLoop* getEventLoopOfCurrentThread();

    void loop();

    void assertInLoopThread();
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid();}

    void quit();
    void updateChannel(ChannelPtr channel);
private:

    //void abortNotInLoopThread();
    bool looping_; /*atomic*/
    const pid_t threadId_;

    bool quit_;
    std::unique_ptr<Poller> poller_;
    ChannelPtrVec activeChannels_;
};
