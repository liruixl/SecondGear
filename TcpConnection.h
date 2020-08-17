#pragma once

#include <memory>

#include "net/InetAddress.h"
#include "utils/Callbacks.h"

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

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    /// Internal use only.

    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once

private:
    //ing与ed的状态在于是否已经将connfd注册到poll/epoll中
    enum StateE {kConnecting, kConnected, };

    void setState(StateE s) { state_ = s; }
    void handleRead();

private:
    EventLoop* loop_;
    std::string name_;
    StateE state_; //FIXME: use atomic variable

    std::unique_ptr<Socket> socket_; //connfd 好像没什么用，只RAII作用
    std::shared_ptr<Channel> channel_; //connfd wrap

    InetAddress localAddr_;
    InetAddress peerAddr_;


    //这两个是干什么的，由TCPServer传递过来
    ConnectionCallback connectionCallback_;  //？ 主动调用connectEstablished
    MessageCallback messageCallback_;  //connfd读事件 即message到来
};