#pragma once

#include <iostream>
#include <memory>
#include <assert.h>

#include "utils/nocopyable.h"
#include "thread/CurrentThread.h"
#include "thread/Thread.h"

#include "Channel.h"
#include "TimerQueue.h"
#include "utils/Callbacks.h"

class Poller;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();
    static EventLoop* getEventLoopOfCurrentThread();

    void loop();

    void assertInLoopThread();
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid();}

    void quit();
    void updateChannel(ChannelPtr channel);
    void removeChannel(ChannelPtr channel);


    ///
    ///允许跨线程使用，添加定时任务
    ///但TimerQueue不是线程安全的，设计上只能在所属IO线程调用
    ///接口内也都断言loop_->assertInLoopThread
    ///如果不设置断言，可以给timerQueue_加锁，在run**函数使用
    ///?那如果既不加锁并且只能在IO线程调用?
    ///只能将TimerQueue::addTimer移动到IO线程进行回调====>runInLoop
    ///
    ///addTimer 将 addTimerInLoop 移动到IO线程调用
    ///这样无论哪个线程调用addTIimer都是安全的了
    ///
    TimerId runAt(const Timestamp& time, const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval, const TimerCallback& cb);

    void runInLoop(const Functor& cb);
    void queueInLoop(const Functor& cb);
private:

    //void abortNotInLoopThread();
    void wakeup();     //eventfd如何使用
    void handleRead(); //wake up callback
    void doPendingFunctors(); //only called by this loop

private:
    bool looping_; /*atomic*/
    const pid_t threadId_;

    ///
    ///Reactor basis
    ///
    bool quit_; /*atomic*/
    std::unique_ptr<Poller> poller_;
    ChannelPtrVec activeChannels_;

    ///
    ///TimerManager : timerfd
    ///
    std::unique_ptr<TimerQueue> timerQueue_; //timerfd


    ///
    ///add Functor & ensure run in this IO Loop
    ///20200813 目前还没有用到其他线程 只用到了IO线程
    ///
    bool callingPendingFunctors_; /*atomic what to do? 有竞态嘛*/
    int wakeupFd_;
    ChannelPtr wakeupChannel_; //Unlike TimerQueue that don't expose Channel to clien

    MutexLock mutex_;
    std::vector<Functor> pendingFunctors; //thread safe mutex
};
