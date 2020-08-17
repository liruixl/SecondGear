#pragma once

//Channel封装fd,及其处理方式
//Poller封装poll,和
#include <vector>
#include <map>

#include "Channel.h"
#include "EventLoop.h"
#include "utils/nocopyable.h"

struct pollfd; 
//class Channel; //思考: 前向申明，还能声明std::shared<Channel>嘛

//不拥有Channel objects
class Poller : noncopyable
{
public:

    Poller(EventLoop* loop);
    ~Poller();

    //返回就绪fd事件
    //Must be called in the loop thread
    ChannelPtrVec poll(int timeoutMs); //for Poller

    //Must be called in the loop thread
    void updateChannel(ChannelPtr channel); //for Poller
    //void removeChannel(ChannelPtr channel);


    void assertInLoopThread() { ownerloop_->assertInLoopThread(); }

private:

    ChannelPtrVec getActiveChannels(int numEvents); //ready fd
    
    EventLoop* ownerloop_;
    std::vector<struct pollfd> pollfds_;  //cache poollfd*
    //std::vector<epoll_event> events_;

    //fd to Channelobj
    //can be a vector(MAXFDS), 空间换时间, O(1) 插入删除
    std::map<int, ChannelPtr> channels_;
};