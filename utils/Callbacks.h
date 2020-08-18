#pragma once

#include <functional>
#include <memory>

// All client visible callbacks go here.


class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

using TimerCallback = std::function<void()> ; //Timer TimerQueue


/*
    TcpServer
*/
using ConnectionCallback =  std::function<void (const TcpConnectionPtr&)>;
using MessageCallback = std::function<void (const TcpConnectionPtr&, const char* data, ssize_t len)> ;
using CloseCallback =  std::function<void (const TcpConnectionPtr&)>;