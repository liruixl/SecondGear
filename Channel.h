#pragma once

#include <functional>
#include <memory>
#include <vector>

class EventLoop;

////
//属于一个EventLoop，因此属于一个IO线程
//只负责一个fd的IO事件分发，但它不拥有这个fd
//socket、eventfd、timerfd、signalfd
//不同的IO事件分发给不同的回调
//Channel 的成员函数都只在IO线程调用，更新数据成员无需枷锁
class Channel : public std::enable_shared_from_this<Channel>
{
public:
    using EventCallBack = std::function<void()>;

    Channel(EventLoop* loop, int fd);

    void handleEvent();
    void setReadCallBack(const EventCallBack& cb)
    { readCallback = cb; }
    void setWriteCallBack(const EventCallBack& cb)
    { writeCallback = cb; }
    void setErrorCallBack(const EventCallBack& cb)
    { errorCallback = cb; }

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }
    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void enableReading() { events_ = kReadEvent; update(); }
    void enableWriting() { events_ = kWriteEvent; update(); }
    void disableWrite() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ =kNoneEvent; update(); }

    //for Poller
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }
    EventLoop* ownerLoop() { return loop_; }
private:
    void update(); //改变events时调用

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;


    EventLoop* loop_;
    const int fd_;
    int events_;   //set by user
    int revents_;  //set by Poller
    int index_;    //used by Poller

    EventCallBack readCallback;
    EventCallBack writeCallback;
    EventCallBack errorCallback;

};

using ChannelPtr = std::shared_ptr<Channel>;
using ChannelPtrVec = std::vector<ChannelPtr>;