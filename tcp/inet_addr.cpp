#include "inet_addr.h"
#include <assert.h>
#include <string.h>

InetAddr::InetAddr()
{
    memset(&addr_, 0, sizeof(addr_));
}

InetAddr::InetAddr(const struct sockaddr_in &addr) : addr_(addr)
{
}

InetAddr::InetAddr(const InetAddr &addr) : addr_(addr.addr_)
{
}

InetAddr::InetAddr(const std::string &ip, uint16_t port)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_aton(ip.c_str(), &addr_.sin_addr);
}

InetAddr::InetAddr(const char *ip, uint16_t port)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_aton(ip, &addr_.sin_addr);
}

InetAddr::InetAddr(uint32_t ip, uint16_t port)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(ip);
}

InetAddr::InetAddr(uint16_t port)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

std::string InetAddr::Ip() const
{
    char *buf = inet_ntoa(addr_.sin_addr);
    assert(buf != NULL);
    return buf;
}

std::string InetAddr::IpPort() const
{
    return Ip() + ":" + std::to_string(static_cast<unsigned long long>(HostOrderPort()));
}

uint32_t InetAddr::HostOrderIp() const
{
    uint32_t ip = ntohl(addr_.sin_addr.s_addr);
    return ip;
}

uint16_t InetAddr::HostOrderPort() const
{
    uint16_t port = ntohs(addr_.sin_port);
    return port;
}

uint32_t InetAddr::NetworkOrderIp() const
{
    return addr_.sin_addr.s_addr;
}

uint16_t InetAddr::NetworkOrderPort() const
{
    return addr_.sin_port;
}
