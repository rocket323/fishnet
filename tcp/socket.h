#ifndef _NET_SOCKET_H_
#define _NET_SOCKET_H_

class InetAddr;

namespace sockets
{
int CreateStreamSocket();
int Bind(int sockfd, const InetAddr &bind_addr);
int CreateNonBlockingStreamSocket();
bool SetNonBlocking(int sockfd, bool on);
bool SetNoDelay(int sockfd, bool on);
// bool SetQuickAck(int sockfd, bool on);
bool SetReuseAddr(int sockfd, bool on);
bool SetKeepAlive(int sockfd, bool on);
bool SetRecvBuffSize(int sockfd, int size);
bool SetSendBuffSize(int sockfd, int size);
int GetSocketError(int sockfd, int &saved_error);

InetAddr GetLocalAddr(int sockfd);
InetAddr GetPeerAddr(int sockfd);
}  // namespace sockets

class Socket
{
public:
    Socket(int sockfd);
    ~Socket();

    int Fd() const { return sock_fd_; }

    // for acceptor
    bool BindAndListen(const InetAddr &addr, int backlog);
    int Accept();

    bool SetNonBlocking(bool on) { return sockets::SetNonBlocking(sock_fd_, on); }
    bool SetNoDelay(bool on) { return sockets::SetNoDelay(sock_fd_, on); }
    // bool SetQuickAck(bool on) { return sockets::SetQuickAck(sock_fd_, on); }
    bool SetReuseAddr(bool on) { return sockets::SetReuseAddr(sock_fd_, on); }
    bool SetKeepAlive(bool on) { return sockets::SetKeepAlive(sock_fd_, on); }
    bool SetRecvBuffSize(int size) { return sockets::SetRecvBuffSize(sock_fd_, size); }
    bool SetSendBuffSize(int size) { return sockets::SetSendBuffSize(sock_fd_, size); }

private:
    const int sock_fd_;
};

#endif
