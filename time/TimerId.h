#pragma once

class Timer;

///
/// An opaque identifier, for canceling Timer.
/// 不透明的标识符，用于取消Timer。实际上就是Timer的地址
///
class TimerId
{
 public:
  explicit TimerId(Timer* timer)
    : value_(timer)
  {
  }

  // default copy-ctor, dtor and assignment are okay

 private:
  Timer* value_;
};