#ifndef _MUTEX_LOCK_H_
#define _MUTEX_LOCK_H_

#include <pthread.h>
#include "nocopyable.h"

class MutexLock : noncopyable
{
public:
    MutexLock() { pthread_mutex_init(&mutex, NULL); }
    ~MutexLock() { pthread_mutex_destroy(&mutex); }

    MutexLock(const MutexLock&) = delete;
    

    void lock() { pthread_mutex_lock(&mutex); }
    void unlock() { pthread_mutex_unlock(&mutex); }

    pthread_mutex_t* get() { return &mutex; }

private:
    pthread_mutex_t mutex;
};


class MutexGuard : noncopyable
{   
public:
    explicit MutexGuard(MutexLock& mutex) 
        : mutex(mutex)
    {
        mutex.lock();
    }
    ~MutexGuard() { mutex.unlock(); } 

private:
    MutexLock & mutex;
};

#define MutexGuard(x) static_assert(false, "missing mutex guard var name")

#endif