#include "connector.h"
#include <unistd.h>
#include "event_loop.h"
#include "inet_addr.h"
#include "socket.h"
#include "tcp_connection.h"

int Connector::Connect(const InetAddr &server_addr, int &sockfd)
{
    sockfd = sockets::CreateNonBlockingStreamSocket();
    if (sockfd < 0)
        return NET_ERR;

    if (::connect(sockfd, server_addr.SockAddr(), sizeof(*server_addr.SockAddr())))
    {
        if (errno == EINPROGRESS)
        {
            // it is ok.
        }
        else
        {
            // error
            ::close(sockfd);
            return NET_ERR;
        }
    }
    return NET_OK;
}

std::shared_ptr<TcpConnection> Connector::Connect(EventLoop *event_loop, const InetAddr &server_addr)
{
    int sockfd;
    int ret = Connect(server_addr, sockfd);
    if (ret != NET_OK)
        return std::shared_ptr<TcpConnection>();

    auto conn = std::make_shared<TcpConnection>(event_loop, sockfd, sockets::GetLocalAddr(sockfd),
                                                sockets::GetPeerAddr(sockfd));

    conn->state_ = TcpConnection::ConnState_Connecting;
    conn->EnableWriting();
    return conn;
}
