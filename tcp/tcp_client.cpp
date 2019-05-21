#include "tcp_client.h"
#include "connector.h"

TcpClient::TcpClient(EventLoop *event_loop, const InetAddr &server_addr)
    : event_loop_(event_loop), server_addr_(server_addr)
{
}

TcpClient::~TcpClient()
{
}

bool TcpClient::Connect(const ConnectionCallback &cb)
{
    connector_.reset(new Connector());
    conn_ = connector_->Connect(event_loop_, server_addr_);
    if (!conn_)
        return false;

    conn_->SetConnectionCallback(cb);
    conn_->EnableWriting();
    return true;
}
