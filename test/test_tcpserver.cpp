#include "EventLoop.h"
#include "stdio.h"
#include <unistd.h>

#include "net/SocketsOps.h"

#include "TcpServer.h"
#include "net/InetAddress.h"
#include "TcpConnection.h"

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
                conn->name().c_str(),
                conn->peerAddress().toHostPort().c_str());
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",
                conn->name().c_str());
    }
}

void onMessage(const TcpConnectionPtr& conn,
            const char* data,
            ssize_t len)
{
    printf("onMessage(): received %zd bytes from connection [%s]\n",
            len, conn->name().c_str());
}


int main()
{
    EventLoop loop;


    InetAddress lisetnAddr(9981);

    TcpServer server(&loop, lisetnAddr, "liruiServer");
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    server.start();
    server.start();
    loop.loop();
}

