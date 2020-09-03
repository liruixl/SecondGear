#include "TcpConnection.h"

#include "net/Socket.h"
#include "EventLoop.h"
#include "Channel.h"
#include "net/SocketsOps.h"

#include "unistd.h"
#include <errno.h>
#include <stdio.h>

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

    auto tcpConnfdReadCallback = [this](Timestamp t){ this->handleRead(t); };
    
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

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    //assert(state_ == kConnected);
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());


    //在这里移除channel -> loop ->Poller
    //loop_->removeChannel(get_pointer(channel_));
    loop_->removeChannel(channel_); //每个类都能通过持有的loop_指针调用其方法
}


void TcpConnection::handleRead(Timestamp receiveTime)
{
    // char buf[65535];
    // ssize_t n = ::read(channel_->fd(),buf, sizeof buf);
    // change to use Buffer

    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);

    //假如需要处理read的数据
    //比如http数据
    //这时候在哪里处理，依然在messageCallback_里处理吗
    //但是回调函数不能保存历史状态啊

    //所以还应该在TcpConnection保存http的状态解析
    //或是加入中间层

    if(n > 0){
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if(n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        std::cout << "TcpConnection::handleRead SYSERR" << std::endl;
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(channel_->isWriting()) {
        ssize_t n = ::write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if(n > 0) {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if(state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            } else {
                std::cout << "I am going to write more data" << std::endl;
            }
        } else {
            std::cerr << "TcpConnection::handleWrite SYSERR" << std::endl;
        }
    } else {
        std::cout << "Connection is down, no more writing" << std::endl;
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    std::cout << "TcpConnection::handleClose state = " << state_ << std::endl;
    //assert(state_ == kConnected);
     assert(state_ == kConnected || state_ == kDisconnecting);

    // we don't close fd, leave it to dtor, so we can find leaks easily.
    channel_->disableAll();
    // must be the last line
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());

    std::cerr << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " "  << std::endl; //<< strerror_tl(err)
}

void TcpConnection::send(const std::string& message)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(message);
        } else {
            auto sendMessageFun = [this, &message](){
                this->sendInLoop(message);
            };
            loop_->runInLoop(sendMessageFun);
        }
    }
}

void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);

        auto shutdownWR = [this](){
            this->shutdownInLoop();
        };

        loop_->runInLoop(shutdownWR);
    }
}

void TcpConnection::sendInLoop(const std::string& message)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;

    //if no thing in output queue, try writing directly
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if(nwrote >= 0){
            if(static_cast<size_t>(nwrote) < message.size()){
                std::cout << "I am going to write more data" << std::endl;
            }
        } else {
            nwrote = 0;
            if(errno != EWOULDBLOCK){
                std::cerr << "TcpConnection::sendInLoop SYSERR" << std::endl;
            }
        }
    }

    assert(nwrote >= 0);
    if(static_cast<size_t>(nwrote) < message.size()) {
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
        if(!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}