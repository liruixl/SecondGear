#pragma once

#include <netinet/in.h>
#include <string>


class InetAddress
{
public:
    /// Constructs an endpoint with given port number.
    /// Mostly used in TcpServer listening.
    ///那端口、地址族呢：端口就是INADDR_ANY，地址族都是AF_INET
    ///注意输入是【主机字节序】
    explicit InetAddress(uint16_t port);
    
    InetAddress(const std::string& ip, uint16_t port);

    /// Constructs an endpoint with given struct @c sockaddr_in
    /// Mostly used when accepting new connections
    /// new connections 客户端地址？
    InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
    { }

    
    std::string toHostPort() const;

    // default copy/assignment are Okay

    const struct sockaddr_in& getSockAddrInet() const { return addr_; }
    void setSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

private:
    sockaddr_in addr_; //sa_family port addr
};