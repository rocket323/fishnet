#ifndef _NET_TCP_CONNECTION_H_
#define _NET_TCP_CONNECTION_H_

#include <memory>
#include "buffer.h"
#include "callbacks.h"
#include "inet_addr.h"

class EventLoop;
class Eventor;
class Socket;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    enum ConnState {
        ConnState_Connecting,
        ConnState_Connected,
        ConnState_Disconnected,
    };

    TcpConnection(EventLoop *event_loop, int sockfd, const InetAddr &local_addr,
                  const InetAddr &peer_addr);
    ~TcpConnection();

    void SetConnectionCallback(const ConnectionCallback &cb) { connection_callback_ = cb; }
    void SetCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }
    void SetReadCallback(const ReadCallback &cb) { read_callback_ = cb; }

    uint64_t Id() const { return conn_id_; }
    const InetAddr &LocalAddr() const { return local_addr_; }
    const InetAddr &PeerAddr() const { return peer_addr_; }

    // Call when connection state become ConnState_Connected.
    void OnConnectionEstablished();

    // Run callback when the read data's length >= read_bytes.
    void ReadBytes(size_t read_bytes, const ReadCallback &cb);
    // Run callback when the read data contain delimiter.
    void ReadUntil(const std::string &delimiter, const ReadCallback &cb);
    // Run callback when the read data's length >= 1
    void ReadAny(const ReadCallback &cb);

    bool Write(const std::string &str);
    bool Write(const char *data, size_t len);
    bool Write(const std::string &str, const WriteCompleteCallback &cb);
    bool Write(const char *data, size_t len, const WriteCompleteCallback &cb);

    void Close();
    void CloseAfter(int64_t delay_ms);
    bool Closed() const { return state_ == ConnState_Disconnected; }

    void EnableReading();
    void DisableReading();
    void EnableWriting();
    void DisableWriting();

    time_t LastActiveTime() const { return last_active_time_; }
    const std::string &ErrMsg() const { return err_msg_; }
    ConnState GetState() const { return state_; }
    EventLoop *GetEventLoop() { return event_loop_; }

    friend class Connector;

private:
    void OnRead();

    void HandleEvents(int revents);
    int HandleConnect();
    void HandleRead();
    void HandleWrite();
    void HandleError();
    void HandleClose();

private:
    // noncopyable
    TcpConnection(const TcpConnection &) = delete;
    TcpConnection &operator=(const TcpConnection &) = delete;

private:
    // atomic
    static uint64_t next_conn_id_;

    EventLoop *event_loop_;
    const uint64_t conn_id_;

    std::unique_ptr<Socket> sock_;
    std::unique_ptr<Eventor> eventor_;

    InetAddr local_addr_;
    InetAddr peer_addr_;

    ConnState state_;
    Buffer input_buffer_;
    Buffer output_buffer_;

    std::string read_delimiter_;
    int read_bytes_;

    time_t last_active_time_;

    ConnectionCallback connection_callback_;
    CloseCallback close_callback_;
    ReadCallback read_callback_;
    WriteCompleteCallback write_complete_callback_;

    std::string err_msg_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::weak_ptr<TcpConnection> TcpConnectionWPtr;

#endif
