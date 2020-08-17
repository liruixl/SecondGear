#include "Acceptor.h"

#include "EventLoop.h"

#include "net/SocketsOps.h"

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


void Acceptor::listen()
{
    //
    //开启监听, 文件描述符放入pollfds中，Channel::update
    //
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_->enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();

    InetAddress peerAddr(0);

    //FIXME loop until no more
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >= 0)
    {
        if(newConnectionCallback_){
            newConnectionCallback_(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    }
    
}