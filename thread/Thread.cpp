#include "Thread.h"
#include <assert.h>
#include <unistd.h> //syscall
#include <sys/syscall.h> //系统调用所使用的符号常量
//#include <linux/unistd.h>

#include <sys/prctl.h>

#include "CurrentThread.h"

namespace CurrentThread
{
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char * t_threadName = "default";
}

pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

void CurrentThread::cacheTid()
{
    if(t_cachedTid == 0)
    {
        t_cachedTid = gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof(t_tidString), "%5d ", t_cachedTid);
    }
}

struct ThreadData
{
    using ThreadFunc = Thread::ThreadFunc;
    ThreadFunc func;
    std::string name;

    //只有在线程创建之后才能知道tid，故这里用指针指向Thraed::tid进行赋值
    pid_t* tid;
    CountDownLatch* latch;

    ThreadData(const ThreadFunc& f, const std::string& name, 
            pid_t* tid, CountDownLatch* latch)
        :func(f), name(name),tid(tid),latch(latch)
    { }

    void runInThread()
    {
        *tid = CurrentThread::tid(); //cache or gettid
        tid = nullptr;
        latch->countDown(); // ?self count down? where wait()
        latch = NULL;

        CurrentThread::t_threadName = name.empty()? "Thread" : name.c_str();
        /* Control process execution.  */
        //把参数arg2作为调用进程的经常名字
        prctl(PR_SET_NAME, CurrentThread::t_threadName);

        func();

        CurrentThread::t_threadName = "finished";// ? name or state?
    }
};


void* startThread(void* arg) //最好是堆对象，生命周期由线程控制
{
    ThreadData* data = static_cast<ThreadData*>(arg);
    data->runInThread();
    delete data;
    return NULL;
}

Thread::Thread(const ThreadFunc& func, const std::string& name)
:started(false),joined(false),
pthreadId(0),tid(0),name(name),
func(func),
latch(1)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started && !joined) pthread_detach(pthreadId);
}

void Thread::start()
{
    assert(!started);
    started = true;
    ThreadData* data = new ThreadData(func,name,&tid,&latch);
    /*
    因为pthread并非Linux系统的默认库，而是POSIX线程库。
    在Linux中将其作为一个库来使用，因此加上 -lpthread（或-pthread）以显式链接该库。
    函数在执行错误时的错误信息将作为返回值返回，
    并不修改系统全局变量errno，当然也无法使用perror()打印错误信息。

    若线程创建成功，则返回0。若线程创建失败，则返回出错编号
    */
    if(pthread_create(&pthreadId, NULL, startThread, data))//&startThread
    {//fail
        started = false;
        delete data;
    } 
    else
    {//ok
        latch.wait(); //wait set tid by syscall
        assert(tid > 0);
        //如果不等待，则调用start之后，tid并没有被设置
        //若此时外部用户访问tid，则薛定谔的猫
    }
}
int Thread::join()
{
    assert(started);
    assert(!joined);
    joined = true;
    return pthread_join(this->pthreadId, NULL);
}

void Thread::setDefaultName()
{
    if(name.empty())
    {
        name = "Thread";
    }
}