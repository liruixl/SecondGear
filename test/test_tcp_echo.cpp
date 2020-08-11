#include "EventLoop.h"
#include "Channel.h"
#include <functional>
#include <iostream>

#include <unistd.h>  //getpid close

#include <sys/timerfd.h>
#include <string.h> //bzero

#include <sys/socket.h>
#include <bits/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

EventLoop* g_loop;

void conn_event(int connfd)
{
    printf("%d recv mesg\n", connfd);

    char mesg[256] = {0};
    int n = recv(connfd,  mesg, sizeof(mesg) - 1 , 0);

    printf("recv %d char, %s\n", n, mesg);
}

int main()
{
    
    EventLoop loop;
    g_loop = &loop;

    int port = 12345;

    //1 创建
    int sock = socket( PF_INET, SOCK_STREAM, 0 );
    assert( sock >= 0 );
    struct sockaddr_in address;
    bzero( &address, sizeof( address ) );
    address.sin_family = AF_INET;
    //inet_pton( AF_INET, ip, &address.sin_addr );
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons( port );
    //2 命名
    int ret = bind( sock, ( struct sockaddr* )&address, sizeof( address ) );
    assert( ret != -1 );
    //3 监听
    ret = listen( sock, 5 );
    assert( ret != -1 );

    auto f = [sock, &loop](){
        sockaddr_in client_addr;
        socklen_t client_addrlength = sizeof(client_addr);

        int connfd = accept(sock, (sockaddr *)&client_addr, &client_addrlength);

        printf("accept a new conn\n");

        ChannelPtr connch = std::make_shared<Channel>(&loop, connfd);
        connch->enableReading();
        connch->setReadCallBack(std::bind(conn_event, connfd));
    };

    printf("listen socket to add\n");
    ChannelPtr ch_listen = std::make_shared<Channel>(&loop, sock);
    ch_listen->enableReading();
    ch_listen->setReadCallBack(f);

    printf("listen socket to add end\n");

    loop.loop();

    close(sock);

    return 0;
}