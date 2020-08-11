#pragma once
#include <functional>
#include <memory>

#include "Timestamp.h"
#include "nocopyable.h"

/*Node
一般来说，定时器结点类持有连接类
到期处理连接，这里抽象为回调函数
TimerManager到期调用run
*/

///
/// Internal class for timer event.
///
class Timer : noncopyable
{
public:
    using TimerCallBack = std::function<void()>;

    Timer(const TimerCallBack& cb, Timestamp when, double interval);

    void run() const
    {
        callback_();
    }

    Timestamp expiration() const  { return expiration_; }
    bool repeat() const { return repeat_; }

    void restart(Timestamp now);


private:
    const TimerCallBack callback_;
    Timestamp expiration_;
    const double interval_;  //what 周期性定时时间.
    const bool repeat_;
};


using TimerPtr = std::shared_ptr<Timer>;