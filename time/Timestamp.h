////copy from https://github.com/chenshuo/recipes/blob/master/datetime/Timestamp.h
////
#pragma once

#include "stdint.h"
#include <string>

class Timestamp
{
private:
    int64_t microSecondsSinceEpoch_;
public:
    Timestamp();
     explicit Timestamp(int64_t microSecondsSinceEpoch);

    void swap(Timestamp& that)
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    // default copy/assignment/dtor are Okay

    std::string toString() const;
    std::string toFormattedString() const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }


    static Timestamp now();

    static Timestamp invalid();

    static const int kMicroSecondsPerSecond = 1000 * 1000;

};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

///
/// Gets time difference of two timestamps, result in seconds.
///
/// @param high, low
/// @return (high-low) in seconds
/// @c double has 52-bit precision, enough for one-microseciond
/// resolution for next 100 years.
inline double timeDifference(Timestamp high, Timestamp low)
{
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

///
/// Add @c seconds to given timestamp.
///
/// @return timestamp+seconds as Timestamp
///
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

