#include "Acceptor.h"

#include "EventLoop.h"

#include "SocketsOps.h"

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
:loop_(loop),
acceptSocket_(sockets::createNonblockingOrDie()),
acceptChannel_(new Channel(loop_, acceptSocket_.fd())),
listenning_(false)
{
    
    //创建绑定
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);

    auto listenfdReadCallback = [this](){
        this->handleRead();
    };
    acceptChannel_->setReadCallBack(listenfdReadCallback);
    
}


void listen()
{

}

void handleRead()
{

}