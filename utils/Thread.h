#pragma once

#include <pthread.h>
#include <functional>
#include <string>

#include "CountDownLatch.h"

/***
Linux thread identifier?
pthread_t : 不一定是一个数值类型，可能是个结构体，glibc的Pthread实现为一个结构体指针(unsigned long)
1 不知道确切类型，无法打印、日志
2 无法比较大小、hash、关联容器的key
3 只在进程内有意义，与操作系统调度之间无法及案例有效关系
4 

Linux getpid(2), 此调用返回的线程ID与POSIX线程ID不同（即pthread_self（3）返回的不透明值）。
1 小整数 int，便于日志输出
2 表示内核任务调度id，/proc/tid找到对应项，利用top命令找出CPU使用率最高的线程id
3 任何时刻是全局唯一的 轮回递增
4 0是非法值，因为操作系统第一个进程init的pid是1
参考解释：https://blog.csdn.net/delphiwcdj/article/details/8476547

```
#include <sys/syscall.h>
printf("The ID of this thread is: %ld\n", (long int)syscall(224));
```
*/

class Thread
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(const ThreadFunc& func, const std::string& name = std::string());
    ~Thread();

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    void start();
    int join();

    bool isStarted() { return started; }
    bool isJoined() { return joined; }
    pid_t getTid() { return tid;} 
    const std::string& getName() { return name; }

private:
    void setDefaultName();
    bool started;
    bool joined;

    pthread_t pthreadId;
    pid_t tid;
    std::string name;

    ThreadFunc func;

    CountDownLatch latch;
};