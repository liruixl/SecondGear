#include "EventLoop.h"
#include "stdio.h"
#include <unistd.h>

#include "Acceptor.h"
#include "net/SocketsOps.h"


void newConn(int sockfd, const InetAddress& peerAddr)
{
    printf("newConn(): accept a new connection from %s\n", peerAddr.toHostPort().c_str());
    ::write(sockfd, "How are you?\n", 13);
    sockets::close(sockfd);
}


//client 
//telnet localhost 9981

int main()
{
    printf("mian():pid = %d\n", getpid());

    InetAddress listenAddr(9981);
    EventLoop loop;

    Acceptor acceptor(&loop, listenAddr);
    acceptor.setNewConnectionCallback(newConn);
    acceptor.listen();

    loop.loop();
}

