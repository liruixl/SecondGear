#pragma once

#include "MutexLock.h"
#include "Condition.h"

class CountDownLatch
{

public:
    explicit CountDownLatch(int count);
    CountDownLatch(const CountDownLatch&) = delete;
    CountDownLatch& operator=(const CountDownLatch&) = delete;

    void wait();
    void countDown();
private:
    mutable MutexLock mutex; //1
    Condition cond;  //2
    int count;
};