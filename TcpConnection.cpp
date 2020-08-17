#include "TcpConnection.h"

#include "net/Socket.h"
#include "EventLoop.h"
#include "Channel.h"
#include "net/SocketsOps.h"

#include "unistd.h"


TcpConnection::TcpConnection(EventLoop* loop,
            const std::string& name,
            int sockfd,
            const InetAddress& localAddr,
            const InetAddress& peerAddr)
:loop_(loop),
name_(name),
state_(kConnecting),
socket_(new Socket(sockfd)),
channel_(new Channel(loop_, sockfd)),
localAddr_(localAddr),
peerAddr_(peerAddr)
{
    std::cout << "TcpConnection::ctor[" <<  name_ << "] at " << this
        << " fd=" << sockfd << std::endl;

    auto tcpConnfdReadCallback = [this](){ this->handleRead(); };
    
    channel_->setReadCallBack(tcpConnfdReadCallback);
}

TcpConnection::~TcpConnection()
{
    std::cout << "TcpConnection::dtor[" <<  name_ << "] at " << this
            << " fd=" << channel_->fd() << std::endl;
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead()
{
    char buf[65535];

    ssize_t n = ::read(channel_->fd(),buf, sizeof buf);


    //假如需要处理read的数据
    //比如http数据
    //这时候在哪里处理，依然在messageCallback_里处理吗
    //但是回调函数不能保存历史状态啊

    //所以还应该在TcpConnection保存http的状态解析
    messageCallback_(shared_from_this(), buf, n);
    
    if(n == 0)
    {
        sockets::close(channel_->fd());
    }
}
