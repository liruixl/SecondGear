#include "stdio.h"
#include "utils/CountDownLatch.h"
#include "utils/Thread.h"

#include "EventLoop.h"
#include <functional>
#include <iostream>

#include <unistd.h>  //getpid

// #include <stdlib.h>
// #include <stdio.h>
// #include <sys/types.h>

using namespace std;


void threadFunc()
{
    printf("threadFunc(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    EventLoop loop;
    loop.loop();
}

int main()
{
    printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    EventLoop loop;

    Thread t(threadFunc);
    t.start();

    loop.loop();

    return 0;
}