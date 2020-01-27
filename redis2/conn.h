#ifndef _REDIS_CONN_H_
#define _REDIS_CONN_H_

#include <functional>
#include <memory>
#include <unordered_map>
#include "async.h"
#include "callbacks.h"
#include "event_loop.h"
#include "eventor.h"
#include "hiredis.h"
#include "tcp/inet_addr.h"
#include "tcp/socket.h"
#include "util.h"

struct redisAsyncContext;
struct redisReply;

class RedisConnection;
typedef std::shared_ptr<RedisConnection> RedisConnectionPtr;
typedef std::function<void(redisReply *)> RedisReplyCallback;

class RedisConnection : public std::enable_shared_from_this<RedisConnection> {
public:
    typedef std::function<void(RedisConnectionPtr, int)> ConnectCallback;
    typedef std::function<void(RedisConnectionPtr)> DisconnectCallback;

    struct Request {
        uint64_t seq;
        uint64_t timer_id;
        RedisReplyCallback callback;
    };

    RedisConnection(EventLoop *loop, redisAsyncContext *context);
    ~RedisConnection();

    int Dov(RedisReplyCallback cb, int64_t timeout_ms, const char *fmt, va_list ap);
    int Do(RedisReplyCallback cb, int64_t timeout_ms, const char *fmt, ...);
    int Do(RedisReplyCallback cb, int64_t timeout_ms, int argc, const char **argv,
           const size_t *argvlen = nullptr);

    // Create a new connection to a specific addr
    static RedisConnectionPtr Connect(EventLoop *event_loop, const InetAddr &addr);
    void Close(bool from_hiredis = false);

public:
    // Trivial functions.
    void EnableReading() { eventor_->EnableReading(); }
    void DisableReading() { eventor_->DisableReading(); }
    void EnableWriting() { eventor_->EnableWriting(); }
    void DisableWriting() { eventor_->DisableWriting(); }
    void Remove() { eventor_->Remove(); }

    void SetConnectCallback(const ConnectCallback &cb) { connect_callback_ = cb; }
    void SetDisconnectCallback(const DisconnectCallback &cb) { disconnect_callback_ = cb; }

    RedisConnection::Request NewRequest(RedisReplyCallback cb);

private:
    // Callback from hiredis
    static void OnConnect(const redisAsyncContext *context, int status);
    static void OnDisconnect(const redisAsyncContext *context, int status);
    static void OnReply(redisAsyncContext *context, void *reply, void *privdata);
    static void OnTimeout(RedisConnectionPtr conn, uint64_t seq);

    // Callback from eventor
    void HandleEvents(int events);

private:
    EventLoop *loop_;
    redisAsyncContext *context_;
    std::unique_ptr<Eventor> eventor_;

    ConnectCallback connect_callback_;
    DisconnectCallback disconnect_callback_;

    bool closed_;
    uint64_t cur_seq_;
    std::unordered_map<uint64_t, Request> cache_requests_;
};

#endif
