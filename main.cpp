#include "EventLoop.h"
#include "Channel.h"
#include <functional>
#include <iostream>

#include <unistd.h>  //getpid close

#include <sys/timerfd.h>
#include <string.h> //bzero

using namespace std;

EventLoop* g_loop;

void timeout()
{
    printf("Timeout\n");
    g_loop->quit();
}

void itoa(long int value, char * str)
{
    char cstr[32];

    int len = 0;
    char * cur = cstr;
    while(value)
    {
        int yushu = value % 10;
        value = value / 10;

        (*cur++) = '0' +  yushu;
        len++;
    }

    char * r = str;
    while(len--)
    {
        (*r++) = (*--cur);
    }
}

int main()
{
    
    EventLoop loop;
    g_loop = &loop;

    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    
    ChannelPtr channel = std::make_shared<Channel>(&loop, timerfd);
    channel->setReadCallBack(timeout);
    channel->enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 3;

    timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop(); //用户看不到poll或者epoll_wait

    close(timerfd);



    return 0;
}