#include "stdio.h"
#include "thread/CountDownLatch.h"
#include "thread/Thread.h"

#include "EventLoop.h"
#include <functional>
#include <iostream>

#include <unistd.h>  //getpid

// #include <stdlib.h>
// #include <stdio.h>
// #include <sys/types.h>

using namespace std;



int main()
{
    printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    EventLoop loop;

    auto f = [&loop](){
        loop.loop();
    };

    Thread t(f);
    t.start();
    t.join();


    return 0;
}