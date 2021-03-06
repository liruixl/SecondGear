#pragma once

#include <memory>

#include "net/InetAddress.h"
#include "utils/Callbacks.h"
#include "Buffer.h"

class EventLoop;
class Socket;
class Channel;

//继承 千万别忘了public
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    /// Constructs a TcpConnection with a connected sockfd
    ///
    /// User should not create this object.
    TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& localAddr,
                const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() { return localAddr_; }
    const InetAddress& peerAddress() { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

    //void send(const void* message, size_t len);
    // Thread safe.
    void send(const std::string& message);
    // Thread safe.
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }

    /// Internal use only.

    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    //
    // called when TcpServer has removed me from its map
    // 析构前调用的最后一个函数，通知用户连接断开
    void connectDestroyed();  // should be called only once

private:
    //ing与ed的状态在于是否已经将connfd注册到poll/epoll中
    enum StateE {kConnecting, kConnected, kDisconnecting , kDisconnected, };

    void setState(StateE s) { state_ = s; }
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose(); //调用closecallback, 此回调绑定到TcpServer::removeConnection()
    void handleError(); //不关闭连接，由Socket RAII管理

    void sendInLoop(const std::string& message);
    void shutdownInLoop();

private:
    EventLoop* loop_;
    std::string name_;
    StateE state_; //FIXME: use atomic variable

    std::unique_ptr<Socket> socket_; //connfd 好像没什么用，只RAII作用
    std::shared_ptr<Channel> channel_; //connfd wrap

    InetAddress localAddr_;
    InetAddress peerAddr_;


    //这两个是干什么的，由TCPServer传递过来
    ConnectionCallback connectionCallback_;  //？ 主动调用connectEstablished ? 连接建立和断开都调用
    MessageCallback messageCallback_;  //connfd读事件 即message到来
    CloseCallback closeCallback_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};