///
///尽力而为服务
///
////个人认为一个完备的定时器需要有如下功能：
//1在某一时间点执行某一任务
//2在某段时间后执行某一任务 1 2 同理
//3重复执行某一任务N次，任务间隔时间T
//4取消重复执行的任务
//学习：https://www.cnblogs.com/ailumiyana/p/9942989.html

////线程安全，如果只在当前IO线程调用则无需加锁

#pragma once

#include <memory>
#include <set>
#include <vector>
#include "nocopyable.h"
#include "time/Timestamp.h"
#include "time/Timer.h"
#include "time/TimerId.h"


#include "Channel.h"

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : noncopyable
{   
public:
    using TimerCallBack = std::function<void()>;
    using Entry = std::pair<Timestamp, TimerPtr>;
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    ///线程安全，通常被其他线程调用，那如何保证 定时器队列 的访问安全
    TimerId addTimer(const TimerCallBack& cb, Timestamp when, double interval);
    //void cancel(TimerId timerId); //如何取消定时任务
private:
    //called when timerfd alarms
    void handleRead();
    std::vector<Entry> getExpired(Timestamp now);

    //?
    void reset(/*const*/ std::vector<Entry>& expired, Timestamp now);

    bool insert(TimerPtr timer);

private:
    EventLoop* loop_;
    const int timerfd_;
    
    ChannelPtr timerfdChannel_;

    //Timer list
    std::set<Entry> timers_;
};