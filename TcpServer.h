#pragma once

#include <map>
#include <string>
#include <memory>

#include "utils/Callbacks.h"

///
///管理accept获得的TcpConnection
///内部使用Acceptor来获得新的connfd
///
///定制Acceptor listening sockfd 回调事件：newConnection
///用户指定TcpConnection的两个回调函数：
///1：连接建立 回调
///2：消息到达（connfd读事件） 回调


class EventLoop;
class Acceptor;
class InetAddress;

class TcpServer
{

public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr, 
        const std::string& name = "TcpServer");
    ~TcpServer(); //force out-line, for unique_ptr member

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
    
    //可多次调用
    //thread safe, 可以从其他线程启动？
    void start(); //启动监听

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_  = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

private:
    ///
    ///Accptor::handleRead里调用
    ///accept之后调用此回调，由TcpServer定制
    ///可以由用户定制吗？
    ///
    //not thread safe but in loop
    void newConnection(int connSockfd, const InetAddress& peerAddr); //Acceptor新建连接 回调
    void removeConnection(const TcpConnectionPtr& conn); //每个连接设置close回调

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

    EventLoop* loop_; //the acceptor loop
    const std::string name_;
    
    std::unique_ptr<Acceptor> acceptor_; //listening sockfd

    //原封不动的传递给TcpConnection
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    

    bool started_;
    int nextConnId_; //always in loop thread
    ConnectionMap connections_;
};