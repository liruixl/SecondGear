#include "Timer.h"


Timer::Timer(const TimerCallBack& cb, Timestamp when, double interval)
:callback_(cb),
expiration_(when),
interval_(interval),
repeat_(interval_ > 0.0) //?
{
}

void Timer::restart(Timestamp now)
{
  if (repeat_)
  {
    expiration_ = addTime(now, interval_);
  }
  else
  {
    expiration_ = Timestamp::invalid();
  }
}