#include "TcpServer.h"

#include "EventLoop.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "net/SocketsOps.h"

#include <stdio.h>  // snprintf

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
    const std::string& name)
:loop_(loop),
name_(name),
acceptor_(new Acceptor(loop_, listenAddr)),
started_(false),
nextConnId_(1)
{
    assert(loop != nullptr);

    auto listeningFdReadCallback = [this]
    (int sockfd, const InetAddress& peerAddr)
    {
        //new connection come in
        newConnection(sockfd, peerAddr);
    }; 

    acceptor_->setNewConnectionCallback(listeningFdReadCallback);

    // acceptor_->setNewConnectionCallback(
    // boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{

}

void TcpServer::start()
{
    //线程安全吗
    //会不会runInLoop多次
    //从而多次listen系统调用，虽然都同步到了IO线程
    // ??????????????
    //多次调用listen是安全的吗
    if(!started_)
    {
        started_ = true;
    }

    if(!acceptor_->listenning())
    {
        //这里没有拷贝unique_ptr对象acceptor吧
        //并且TcpServer生命周期比acceptor_长
        auto startListenning = [this](){
            acceptor_ ->listen();
        };

        loop_->runInLoop(startListenning);
    }
}


void TcpServer::newConnection(int connSockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnId_);
    ++nextConnId_;

    std::string connName = name_ + buf;

    std::cout << "TcpServer::newConnection [" << name_
        << "] - new connection [" << connName
        << "] from " << peerAddr.toHostPort() << std::endl;
    
    InetAddress localAddr(sockets::getLocalAddr(connSockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    //TO DO FIX

    // TcpConnectionPtr conn(
    //     new TcpConnection(loop_, connName, connSockfd, localAddr, peerAddr));

    TcpConnectionPtr conn = 
        std::make_shared<TcpConnection>(loop_, connName, connSockfd, localAddr, peerAddr);

    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);

    auto closeCallbackToRemove = [this](const TcpConnectionPtr& conn){
        removeConnection(conn);
    };

    conn->setCloseCallback(closeCallbackToRemove);
    conn->connectEstablished();
}

///
///从TcpServer列表里移除
///从loop Poller中移除
///
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    std::cout << "TcpServer::removeConnection [" << name_
            << "] - connection " << conn->name() << std::endl;
    size_t n = connections_.erase(conn->name());
    assert(n == 1); (void)n;

    auto connectedDestroyed = [conn](){
        conn->connectDestroyed();
    };
    //???????????????????????????????????
    //这里为什么要 queueInloop啊?本来就是在IO线程啊
    loop_->queueInLoop(connectedDestroyed);
    // loop_->queueInLoop(
    //     std::bind(&TcpConnection::connectDestroyed, conn));
}
