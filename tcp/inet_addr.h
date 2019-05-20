#ifndef _INET_ADDR_H_
#define _INET_ADDR_H_

#include <arpa/inet.h>
#include <string>

class InetAddr
{
public:
    InetAddr();
    InetAddr(const struct sockaddr_in &addr);
    InetAddr(const InetAddr &addr);
    InetAddr(const std::string &ip, uint16_t port);
    InetAddr(const char *ip, uint16_t port);
    InetAddr(uint32_t ip, uint16_t port);
    InetAddr(uint16_t port);

    std::string Ip() const;
    std::string IpPort() const;

    uint32_t HostOrderIp() const;
    uint16_t HostOrderPort() const;
    uint32_t NetworkOrderIp() const;
    uint16_t NetworkOrderPort() const;

    struct sockaddr *SockAddr() { return (struct sockaddr *)(&addr_); }
    const struct sockaddr *SockAddr() const { return (const struct sockaddr *)(&addr_); }
    const struct sockaddr_in &SockAddrIn() const { return addr_; }

    operator std::string() const { return IpPort(); }

private:
    struct sockaddr_in addr_;
};

#endif
