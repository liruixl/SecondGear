#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    :mutex(),
    cond(mutex),
    count(count)
{ }


void CountDownLatch::wait()
{
    MutexGuard lock(mutex);
    while(count > 0)
    {
        cond.wait();
    }
}

void CountDownLatch::countDown()
{
    MutexGuard lock(mutex);
    --count;
    if(count == 0)
    {
        cond.notifyAll();
    }
}

