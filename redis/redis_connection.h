#ifndef _NET_REDIS_CONNECTION_H_
#define _NET_REDIS_CONNECTION_H_

#include <stdarg.h>
#include <functional>
#include <list>
#include <memory>
#include "callbacks.h"
#include "event_loop.h"
#include "eventor.h"
#include "tcp/inet_addr.h"
#include "util.h"

struct redisAsyncContext;
struct redisReply;

class RedisConnection;
typedef std::shared_ptr<RedisConnection> RedisConnectionPtr;
typedef std::function<void(redisReply *)> RedisReplyCallback;

// Redis connection wraps redisAsyncContext
class RedisConnection : public std::enable_shared_from_this<RedisConnection>
{
public:
    typedef std::function<void(RedisConnectionPtr, int)> ConnectCallback;
    typedef std::function<void(RedisConnectionPtr)> DisconnectCallback;

    RedisConnection(EventLoop *event_loop, redisAsyncContext *redis_context, const InetAddr &local_addr,
                    const InetAddr &peer_addr);
    ~RedisConnection();

    // Exec redis command
    int Execv(const RedisReplyCallback &cb, const char *fmt, va_list ap);
    int Exec(const RedisReplyCallback &cb, const char *fmt, ...);
    int Exec(const RedisReplyCallback &cb, int argc, const char **argv, const size_t *argvlen);

    // Use for RedisAdapter
    void EnableReading() { eventor_->EnableReading(); }
    void DisableReading() { eventor_->DisableReading(); }
    void EnableWriting() { eventor_->EnableWriting(); }
    void DisableWriting() { eventor_->DisableWriting(); }
    void Remove() { eventor_->RemoveSelf(); }

    void SetConnectCallback(const ConnectCallback &cb) { connect_callback_ = cb; }
    void SetDisconnectCallback(const DisconnectCallback &cb) { disconnect_callback_ = cb; }

    const InetAddr &LocalAddr() const { return local_addr_; }
    const InetAddr &PeerAddr() const { return peer_addr_; }

    uint64_t Id() const { return conn_id_; }
    void Close();
    bool Closed() const { return closed_; }
    bool IdleExpired(int timeout) { return (Util::CurrentSystemTimeMillis() - active_ts_) >= timeout; }

    // Create a new connection connected to specific addr
    static RedisConnectionPtr Connect(EventLoop *event_loop, const InetAddr &addr);

private:
    static void OnConnect(const redisAsyncContext *context, int status);
    static void OnDisconnect(const redisAsyncContext *context, int status);
    static void OnRedisReply(redisAsyncContext *context, void *reply, void *privdata);

    void HandleEvents(int revents);
    void HandleRead();
    void HandleWrite();
    void HandleError();
    void HandleClose();

private:
    static uint64_t next_conn_id_;
    EventLoop *event_loop_;
    const uint64_t conn_id_;
    redisAsyncContext *redis_context_;
    std::unique_ptr<Eventor> eventor_;
    InetAddr local_addr_;
    InetAddr peer_addr_;

    int active_ts_;
    int closed_;

    ConnectCallback connect_callback_;
    DisconnectCallback disconnect_callback_;

    // Reply callbacks for redis request
    std::list<RedisReplyCallback> reply_callbacks_;
};

#endif
