#include "EventLoop.h"
#include "Channel.h"
#include <functional>
#include <iostream>

#include <unistd.h>  //getpid close

#include <sys/timerfd.h>
#include <string.h> //bzero

using namespace std;

EventLoop* g_loop;

void timeout(int i)
{
    printf("Timeout %d\n", i);

    if(i == 3) g_loop->quit(); //3s 后停止
}

int main()
{
    
    EventLoop loop;
    g_loop = &loop;

    vector<int> fds;

    for(int i = 0; i < 10; i++)
    {
        int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    
        ChannelPtr channel = std::make_shared<Channel>(&loop, timerfd);
        auto f = [i](){
            timeout(i + 1);
        };
        channel->setReadCallBack(f);
        channel->enableReading();

        struct itimerspec howlong;
        bzero(&howlong, sizeof howlong);
        howlong.it_value.tv_sec = i + 1;

        timerfd_settime(timerfd, 0, &howlong, NULL);
        fds.push_back(timerfd);
    }

    loop.loop(); //用户看不到poll或者epoll_wait

    for(int fd : fds) close(fd);

    return 0;
}