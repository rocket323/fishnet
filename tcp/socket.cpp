#include "socket.h"
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "inet_addr.h"

Socket::Socket(int sockfd) : sock_fd_(sockfd) {
    assert(sockfd >= 0);
}

Socket::~Socket() {
    ::close(sock_fd_);
}

bool Socket::BindAndListen(const InetAddr &addr, int backlog) {
    socklen_t sockaddr_len = sizeof(addr.SockAddrIn());
    if (::bind(sock_fd_, addr.SockAddr(), sockaddr_len) < 0)
        return false;
    if (::listen(sock_fd_, backlog) < 0)
        return false;
    return true;
}

int Socket::Accept() {
    struct sockaddr_in addr;
    socklen_t sockaddr_len = sizeof(addr);
    int fd = ::accept(sock_fd_, (struct sockaddr *)(&addr), &sockaddr_len);
    return fd;
}

namespace sockets {
int CreateStreamSocket() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    return sockfd;
}

int Bind(int sockfd, const InetAddr &bind_addr) {
    socklen_t sockaddr_len = sizeof(bind_addr.SockAddrIn());
    if (::bind(sockfd, bind_addr.SockAddr(), sockaddr_len) < 0)
        return false;
    return true;
}

int CreateNonBlockingStreamSocket() {
    int sockfd = CreateStreamSocket();
    if (sockfd < 0)
        return sockfd;

    SetNonBlocking(sockfd, true);
    return sockfd;
}

bool SetNonBlocking(int sockfd, bool on) {
    int ret = ::fcntl(sockfd, F_GETFL, 0);
    if (ret < 0)
        return false;

    if (!!(ret & O_NONBLOCK) == on) {
        // already set
        return true;
    }

    if (on)
        ret |= O_NONBLOCK;
    else
        ret &= ~O_NONBLOCK;

    ret = ::fcntl(sockfd, F_SETFL, ret);
    if (ret < 0)
        return false;

    return true;
}

bool SetNoDelay(int sockfd, bool on) {
    int val = on ? 1 : 0;
    if (::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)))
        return false;
    return true;
}

// bool SetQuickAck(int sockfd, bool on)
// {
//     int val = on ? 1 : 0;
//     if (::setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &val, sizeof(val)))
//         return false;
//     return true;
// }

bool SetReuseAddr(int sockfd, bool on) {
    int val = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)))
        return false;
    return true;
}

bool SetKeepAlive(int sockfd, bool on) {
    int val = on ? 1 : 0;
    if (::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)))
        return false;
    return true;
}

bool SetRecvBuffSize(int sockfd, int size) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)))
        return false;
    return true;
}

bool SetSendBuffSize(int sockfd, int size) {
    if (::setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)))
        return false;
    return true;
}

int GetSocketError(int sockfd, int &saved_error) {
    int error = 0;
    socklen_t error_len = sizeof(error);
    int ret = ::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &error_len);
    if (ret == 0)
        saved_error = error;
    return ret;
}

InetAddr GetLocalAddr(int sockfd) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    ::getsockname(sockfd, (struct sockaddr *)&addr, &addr_len);
    return InetAddr(addr);
}

InetAddr GetPeerAddr(int sockfd) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    ::getpeername(sockfd, (struct sockaddr *)&addr, &addr_len);
    return InetAddr(addr);
}
}  // namespace sockets
