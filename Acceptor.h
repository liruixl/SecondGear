#pragma once

#include <functional>

#include "net/InetAddress.h"
#include "Channel.h"
#include "net/Socket.h"


class EventLoop;

//interval class used by TcpServer
class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr);
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;
    
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }
    bool listenning() const { return listenning_; }
    void listen();
private:
    void handleRead();

    EventLoop* loop_;

    Socket acceptSocket_;
    ChannelPtr acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};