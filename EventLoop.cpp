#include "EventLoop.h"

#include <unistd.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include "Poller.h"

__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;
//静态全局函数怎么回事
static int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        printf("Failed in eventfd\n");
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
:looping_(false),
threadId_(CurrentThread::tid()),
quit_(false),
callingPendingFunctors_(false),
wakeupFd_(createEventfd())
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
    poller_ = std::unique_ptr<Poller>(new Poller(this)); //fds
    timerQueue_ = std::unique_ptr<TimerQueue>(new TimerQueue(this)); //timerfd
    wakeupChannel_ = std::unique_ptr<Channel>(new Channel(this, wakeupFd_)); //eventfd
    wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
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
        activeChannels_ = poller_->poll(kPollTimeMs);

        for(auto it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
        {
            (*it)->handleEvent();
        }
    }

    doPendingFunctors();

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


TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
    Timestamp time = addTime(Timestamp::now(), delay);
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    Timestamp time = addTime(Timestamp::now(), interval);
    return timerQueue_->addTimer(cb, time, interval);
}


void EventLoop::runInLoop(const Functor& cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else{
        queueInLoop(cb);
    }
}
void EventLoop::queueInLoop(const Functor& cb)
{
    {
        MutexGuard lock(mutex_);
        pendingFunctors.push_back(std::move(cb));   
    }

    ///
    ///放入队列就可以了嘛
    ///
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}


void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        std::cout << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
    }
}

void EventLoop::handleRead()
{
    //读eventfd，要是不读的话是否重复触发？
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one); //n 代表什么?
    if (n != sizeof one) //8字节无符号
    {
        std::cout << "EventLoop::handleRead() reads " << n << " bytes instead of 8" << std::endl;
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexGuard lock(mutex_);
        functors.swap(pendingFunctors);
    }

    for(int i = 0; i < functors.size(); i++)
    {
        functors[i]();
    }

    callingPendingFunctors_ = false;
}