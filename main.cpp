#include "EventLoop.h"
#include "Channel.h"
#include "TimerQueue.h"
#include <functional>
#include <iostream>

#include <unistd.h>  //getpid close

#include <sys/timerfd.h>
#include <string.h> //bzero


void test()
{

    std::cout << "[test] : test timerQueue" << std::endl;

}

using namespace std;

int main()
{
    
    EventLoop loop;

    TimerQueue timerQue(&loop);
    timerQue.addTimer(test, addTime(Timestamp::now(), 3.0), 3);
    timerQue.addTimer(test, addTime(Timestamp::now(), 3.0), 0);
    timerQue.addTimer(test, addTime(Timestamp::now(), 5.0), 0);

    loop.loop();
    return 0;
}