#ifndef _CONDITION_
#define _CONDITION_

#include <time.h>
#include <errno.h>
//#include <cstdint>

#include "MutexLock.h"
#include "nocopyable.h"

class Condition : noncopyable
{
public:
    Condition(MutexLock& mutex) : mutex(mutex) 
    {
        pthread_cond_init(&cond, NULL);
    }

    ~Condition() { pthread_cond_destroy(&cond); }

    void wait() { pthread_cond_wait(&cond, mutex.get()); }
    void notify() { pthread_cond_signal(&cond); }
    void notifyAll() { pthread_cond_broadcast(&cond); }
    bool waitForSeconds(int seconds) 
    {
        struct timespec interval;
        clock_gettime(CLOCK_REALTIME, &interval);
        interval.tv_sec += static_cast<time_t>(seconds);

        return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &interval);
    }

private:
    MutexLock& mutex;
    pthread_cond_t cond;
};

#endif