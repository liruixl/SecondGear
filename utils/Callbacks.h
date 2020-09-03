#pragma once

#include <functional>
#include <memory>
#include "../time/Timestamp.h"

// All client visible callbacks go here.


class TcpConnection;
class Buffer;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

using TimerCallback = std::function<void()> ; //Timer TimerQueue


/*
    TcpServer
*/
using ConnectionCallback =  std::function<void (const TcpConnectionPtr&)>;
//using MessageCallback = std::function<void (const TcpConnectionPtr&, const char* data, ssize_t len)>;
using MessageCallback = std::function<void (const TcpConnectionPtr&, Buffer* buffer,  Timestamp)>;


using CloseCallback =  std::function<void (const TcpConnectionPtr&)>;