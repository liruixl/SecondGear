#include "EventLoop.h"
#include "Channel.h"
#include "TimerQueue.h"
#include <functional>
#include <iostream>

#include <unistd.h>  //getpid close

#include <sys/timerfd.h>
#include <string.h> //bzero


void test(int timerfd)
{
    uint64_t howmany;

    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);

    std::cout << "[test] : test timerQueue" << std::endl;

}

using namespace std;

int main()
{
    
    EventLoop loop;

    //这里的不会重复触发
    // TimerQueue timerQue(&loop);
    // timerQue.addTimer(test, addTime(Timestamp::now(), 3.0), 0);
    // // timerQue.addTimer(test, addTime(Timestamp::now(), 3.0), 0);
    // // timerQue.addTimer(test, addTime(Timestamp::now(), 5.0), 0);


    //为甚么下面这段代码重复触发timerfd啊 卧槽必须read，不然重复触发。。我人傻了
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);

    ChannelPtr channel = std::make_shared<Channel>(&loop, timerfd);
    auto f = [timerfd]()
    {
        test(timerfd);
    };
    channel->setReadCallBack(f);
    channel->enableReading();
    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 3;
    timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop();
    close(timerfd);
    return 0;
}