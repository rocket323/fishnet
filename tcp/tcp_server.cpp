#include "tcp_server.h"
#include "acceptor.h"
#include "socket.h"
#include "tcp_connection.h"
#include "util.h"

using namespace std::placeholders;

TcpServer::TcpServer(EventLoop *event_loop, const InetAddr &server_addr)
    : event_loop_(event_loop), server_addr_(server_addr)
{
}

TcpServer::~TcpServer()
{
}

bool TcpServer::Start()
{
    acceptor_.reset(new Acceptor(event_loop_, server_addr_, std::bind(&TcpServer::OnAccept, this, _1)));

    bool succ = acceptor_->Listen();
    if (!succ)
    {
        err_msg_ = acceptor_->ErrMsg();
        acceptor_.reset();
    }
    return succ;
}

void TcpServer::Stop()
{
    acceptor_->Stop();
    event_loop_->Stop();
}

void TcpServer::OnAccept(int sockfd)
{
    // New connection
    InetAddr local_addr(sockets::GetLocalAddr(sockfd));
    InetAddr peer_addr(sockets::GetPeerAddr(sockfd));
    TcpConnectionPtr conn(new TcpConnection(event_loop_, sockfd, local_addr, peer_addr));

    // Put conn into conns map
    // TODO: thread-safety
    conns_map_[conn->Id()] = conn;
    conn->SetConnectionCallback(connection_callback_);
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, _1));
    conn->SetReadCallback(read_callback_);

    // Make sure to call OnConnectionEstablished in loop-thread
    conn->GetEventLoop()->Post(std::bind(&TcpConnection::OnConnectionEstablished, conn));
}

void TcpServer::RemoveConnection(TcpConnectionPtr conn)
{
    if (close_callback_)
        close_callback_(conn);

    // TODO: thread-safety
    conns_map_.erase(conn->Id());
}
