#include "TimerQueue.h"
 
#include "EventLoop.h"

#include "sys/timerfd.h"
#include <unistd.h> //read
#include "string.h"

#include <iostream>
#include <memory>

namespace timerpackage
{
int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                    TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        printf("Failed in timerfd_create\n");
    }
    assert(timerfd >= 0);
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch()
                            - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    //可以用read函数读取计时器的超时次数，该值是一个8字节无符号的长整型
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    std::cout << "TimerQueue::handleRead() read " << howmany << " at " << now.toString() << std::endl;
    if (n != sizeof howmany)
    {
        std::cout << "TimerQueue::handleRead() reads " << n << " bytes instead of 8" << std::endl;;
    }
}

void resetTimerfd(int timerfd, Timestamp expiration)
{
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    //不设置周期
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        std::cout << "timerfd_settime()" << std::endl;
    }
}

}//namespace timerpackge

using namespace timerpackage;


TimerQueue::TimerQueue(EventLoop* loop)
:loop_(loop),
timerfd_(createTimerfd()),
timerfdChannel_(new Channel(loop_, timerfd_)),
timers_()
{
    //timerfdChannel_ = std::make_shared<Channel>(loop_, timerfd_);
    timerfdChannel_->setReadCallBack(std::bind(&TimerQueue::handleRead, this));
    timerfdChannel_->enableReading();
}
TimerQueue::~TimerQueue()
{
    ::close(timerfd_);
}

TimerId TimerQueue::addTimer(const TimerCallBack& cb, Timestamp when, double interval)
{
    //C++14
    //TimerUPtr timer = std::make_unique<Timer>(cb, when, interval);
    TimerPtr timer(new Timer(cb,when,interval));
    TimerId timerid(timer.get());

    auto moveAddTimerToIO = [&](){
        this->addTimerInLoop(timer);
    };
    
    loop_->runInLoop(moveAddTimerToIO);

    return timerid;
}

void TimerQueue::addTimerInLoop(TimerPtr timer)
{
    loop_->assertInLoopThread();

    bool earliestChanged = insert(timer); //std::move(timer)段错误

    //把定时器的超时时间设置为第一个early的超时时间
    //之前是惰性删除，固定时钟周期，超时信号到来在IO线程最后处理超时
    //或者不用定时，只在循环的最后处理超时事件
    if (earliestChanged)
    {
        resetTimerfd(timerfd_, timer->expiration()); //段错误发生在这里timer被move
    }
}


void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);

    std::vector<Entry> expired = getExpired(now);

    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
        it != expired.end(); ++it)
    {
        it->second->run();
    }

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    //Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    Entry sentry = {now, TimerPtr()};
    auto it = timers_.lower_bound(sentry); //unique_ptr能比较大小嘛
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, back_inserter(expired));

    // std::copy ( make_move_iterator(timers_.begin()), 
    //             make_move_iterator(it),
    //             back_inserter(expired) );

    // for(auto ii = timers_.begin(); ii != it; ++ii)
    // {
    //     //set 存放的都是键值 拥有const iter, 解引用也是const &类型
    //     expired.push_back(std::move((*ii)));
    //     //对一个const XX& move会发生什么.....
    //      能移动嘛
    // }

    timers_.erase(timers_.begin(), it);

    return expired;
}


void TimerQueue::reset(/*const*/ std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;

    //注意这里auto的推断是const 因为参数是const
    for (auto it = expired.begin();it != expired.end(); ++it)
    {
        if (it->second->repeat())
        {
            it->second->restart(now);
            insert(it->second); //重启并加入
        }
        else
        {
            // FIXME move to a free list
            //delete it->second;
        }
    }

    if (!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }

    if (nextExpire.valid())
    {
        resetTimerfd(timerfd_, nextExpire);
    }
}

//如果函数以unique_ptr作为参数呢？
bool TimerQueue::insert(TimerPtr timer)
{
    bool earliestChanged = false; // ?
    Timestamp when = timer->expiration();
    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }

    Entry entry = {timer->expiration(), timer};
    auto result =  timers_.insert(entry);

    assert(result.second);

    return earliestChanged;
}