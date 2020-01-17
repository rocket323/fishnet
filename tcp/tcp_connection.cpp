#include "tcp_connection.h"
#include <assert.h>
#include <unistd.h>
#include "event_loop.h"
#include "eventor.h"
#include "socket.h"
#include "util.h"

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
      last_active_time_(Util::CurrentSystemTime()),
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

void TcpConnection::ReadUntil(const std::string &delimiter, const ReadCallback &cb)
{
    assert(cb);
    assert(!delimiter.empty());
    if (Closed())
        return;
    if (input_buffer_.Find(delimiter) != NULL)
    {
        event_loop_->Post(std::bind(cb, shared_from_this(), &input_buffer_));
        return;
    }
    read_delimiter_ = delimiter;
    read_callback_ = cb;
    if (!eventor_->Reading())
        EnableReading();
}

bool TcpConnection::Write(const std::string &str)
{
    return Write(str.data(), str.length(), NULL);
}

bool TcpConnection::Write(const char *data, size_t len)
{
    return Write(data, len, NULL);
}

bool TcpConnection::Write(const std::string &str, const WriteCompleteCallback &cb)
{
    return Write(str.data(), str.length(), cb);
}

bool TcpConnection::Write(const char *data, size_t len, const WriteCompleteCallback &cb)
{
    // Not allow to send data when closed
    if (Closed())
        return false;

    if (len == 0)
    {
        // It is always ok to send empty data
        if (cb)
            event_loop_->Post(std::bind(cb, shared_from_this()));
        return true;
    }

    // Send data directly when no pending data to send
    if (state_ == ConnState_Connected && output_buffer_.ReadableBytes() == 0)
    {
        int nwrote = ::write(eventor_->Fd(), data, len);
        if (nwrote < 0)
        {
            // Send data failed
            // write data to output buffer and wait for POLLOUT event
        }
        else if (nwrote >= static_cast<int>(len))
        {
            // Whole data was sent directly
            if (cb)
                event_loop_->Post(std::bind(cb, shared_from_this()));
            return true;
        }
        else
        {
            // Data was sent partially
            // write the rest to output buffer
            data += nwrote;
            len -= nwrote;
        }
    }

    // TODO(rocket323): use a list?
    write_complete_callback_ = cb;
    output_buffer_.Append(data, len);
    if (!eventor_->Writing())
        eventor_->EnableWriting();
    return true;
}

void TcpConnection::Close()
{
    HandleClose();
}

void TcpConnection::CloseAfter(int64_t delay_ms)
{
    event_loop_->AssertIsCurrent();
    event_loop_->RunAfter(delay_ms, std::bind(&TcpConnection::Close, shared_from_this()));
}

void TcpConnection::EnableReading()
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return;
    eventor_->EnableReading();
}

void TcpConnection::DisableReading()
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return;
    eventor_->DisableReading();
}

void TcpConnection::EnableWriting()
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return;
    eventor_->EnableWriting();
}

void TcpConnection::DisableWriting()
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return;
    eventor_->DisableWriting();
}

void TcpConnection::OnConnectionEstablished()
{
    event_loop_->AssertIsCurrent();
    assert(state_ == ConnState_Connecting);
    state_ = ConnState_Connected;

    eventor_->EnableReading();
    TcpConnectionPtr conn(shared_from_this());
    connection_callback_(conn);
}

void TcpConnection::OnRead()
{
    ReadCallback read_callback;
    if (input_buffer_.ReadableBytes() > 0)
        read_callback = read_callback_;
    // if (read_bytes_ > 0 && static_cast<int>(input_buffer_.ReadableBytes()) >= read_bytes_)
    // {
    //     // ReadAny or ReadBytes
    //     read_callback = std::move(read_callback_);
    //     read_bytes_ = -1;
    // }
    // else if (!read_delimiter_.empty() && input_buffer_.Find(read_delimiter_) != NULL)
    // {
    //     // ReadUntil
    //     read_callback = std::move(read_callback_);
    //     read_delimiter_.clear();
    // }

    if (read_callback)
        read_callback(shared_from_this(), &input_buffer_);
    return;
}

void TcpConnection::HandleRead()
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return;

    ssize_t nread = input_buffer_.ReadFromFd(sock_->Fd());
    if (nread < 0)
    {
        // Error
        HandleError();
        return;
    }
    else if (nread == 0)
    {
        // Close by peer
        HandleClose();
    }
    else
    {
        OnRead();
    }
}

void TcpConnection::HandleEvents(int revents)
{
    // Prevent connection being destroyed in HandleXXX()
    TcpConnectionPtr guard(shared_from_this());

    last_active_time_ = Util::CurrentSystemTime();

    if (state_ == ConnState_Connecting)
    {
        if (HandleConnect() == NET_OK)
        {
            assert(eventor_->Writing());
            // Async connect success, disable writing when
            if (output_buffer_.ReadableBytes() == 0)
                DisableWriting();

            OnConnectionEstablished();
        }
        else
        {
            // Error
            HandleError();
            return;
        }
    }

    if (revents & Poller::READABLE)
        HandleRead();

    if (revents & Poller::WRITABLE)
        HandleWrite();
}

int TcpConnection::HandleConnect()
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return NET_ERR;

    int saved_error = 0;
    int ret = sockets::GetSocketError(sock_->Fd(), saved_error);
    if (ret || saved_error)
        return NET_ERR;

    return NET_OK;
}

void TcpConnection::HandleWrite()
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return;

    if (output_buffer_.ReadableBytes() == 0)
    {
        DisableWriting();
        return;
    }

    int nwrote = ::write(eventor_->Fd(), output_buffer_.Peek(), output_buffer_.ReadableBytes());
    if (nwrote < 0)
    {
        if (errno == EINTR)
        {
            // It is ok.
        }
        else if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            // It is ok.
        }
        else
        {
            // Error
            HandleError();
            return;
        }
    }
    else
    {
        output_buffer_.Retrieve(nwrote);
    }

    // Write comletely
    if (output_buffer_.ReadableBytes() == 0)
    {
        eventor_->DisableWriting();
        if (write_complete_callback_)
            write_complete_callback_(shared_from_this());
    }
}

void TcpConnection::HandleClose()
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return;
    state_ = ConnState_Disconnected;

    // Remove events from poller
    eventor_->Remove();

    TcpConnectionPtr guard(shared_from_this());

    // Prevent circular references
    connection_callback_ = std::bind(&DefaultConnectionCallback, _1);
    write_complete_callback_ = NULL;
    read_callback_ = NULL;

    close_callback_(guard);
}

void TcpConnection::HandleError()
{
    event_loop_->AssertIsCurrent();
    HandleClose();
}
