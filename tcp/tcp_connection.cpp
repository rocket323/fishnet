#include "tcp_connection.h"
#include <unistd.h>
#include "event_loop.h"
#include "eventor.h"
#include "socket.h"

using namespace std::placeholders;

static void DefaultConnectionCallback(const TcpConnectionPtr &conn)
{
    // do nothing
}

uint64_t TcpConnection::next_conn_id_(1);

TcpConnection::TcpConnection(EventLoop *event_loop, int sockfd, const InetAddr &local_addr, const InetAddr &peer_addr)
    : event_loop_(event_loop),
      conn_id_(__sync_fetch_and_add(&next_conn_id_, 1)),
      sock_(new Socket(sockfd)),
      eventor_(new Eventor(event_loop, sockfd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      state_(ConnState_Connecting),
      read_bytes_(-1),
      last_active_time_(0),
      connection_callback_(std::bind(&DefaultConnectionCallback, _1))
{
    eventor_->SetEventsCallback(std::bind(&TcpConnection::HandleEvents, this, _1));
    sock_->SetKeepAlive(true);
    sock_->SetNonBlocking(true);
}

TcpConnection::~TcpConnection()
{
    // Ensures to release all resources.
    assert(Closed());
}

void TcpConnection::ReadAny(const ReadCallback &cb)
{
    return ReadBytes(1, cb);
}

void TcpConnection::ReadBytes(size_t read_bytes, const ReadCallback &cb)
{
    assert(cb);
    if (Closed())
        return;

    if (input_buffer_.ReadableBytes() >= read_bytes)
    {
        event_loop_->Post(std::bind(cb, shared_from_this(), &input_buffer_));
        return;
    }

    read_bytes_ = read_bytes;
    read_callback_ = cb;
    if (!eventor_->Reading())
        EnableReading();

    return;
}
