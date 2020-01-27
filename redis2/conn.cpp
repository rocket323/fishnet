#include "redis2/conn.h"
#include <assert.h>
#include <cstring>
#include "adapter.h"

using namespace std::placeholders;

RedisConnection::RedisConnection(EventLoop *loop, redisAsyncContext *context)
    : loop_(loop),
      context_(context),
      eventor_(new Eventor(loop, context->c.fd)),
      closed_(false),
      cur_seq_(0) {
    eventor_->SetEventsCallback(std::bind(&RedisConnection::HandleEvents, this, _1));
}

RedisConnection::~RedisConnection() {
    // Help find memory leaks.
    assert(context == nullptr);
}

RedisConnectionPtr RedisConnection::Connect(EventLoop *loop, const InetAddr &addr) {
    RedisConnectionPtr conn;
    redisAsyncContext *context = redisAsyncConnect(addr.Ip().c_str(), addr.HostOrderPort());
    if (context == nullptr)
        return conn;

    if (context->err) {
        redisAsyncFree(context);
        return conn;
    }

    conn = std::make_shared<RedisConnection>(loop, context);
    RedisNetAttach(context, conn.get());
    redisAsyncSetConnectCallback(context, OnConnect);
    redisAsyncSetDisconnectCallback(context, OnDisconnect);

    return conn;
}

RedisConnection::Request RedisConnection::NewRequest(RedisReplyCallback cb) {
    RedisConnection::Request req = {++cur_seq_, 0, cb};
    return req;
}

int RedisConnection::Dov(RedisReplyCallback cb, int64_t timeout_ms, const char *fmt, va_list ap) {
    auto req = NewRequest(cb);
    uint64_t *p_seq = new uint64_t(req.seq);

    int status = redisAsyncCommand(context_, OnReply, p_seq, fmt, ap);
    if (status != REDIS_OK)
        return NET_ERR;

    // Add timer.
    TimerId timer_id = loop_->RunAfter(
        timeout_ms, std::bind(&RedisConnection::OnTimeout, shared_from_this(), req.seq));
    req.timer_id = timer_id;
    cache_requests_[req.seq] = req;

    return NET_OK;
}

int RedisConnection::Do(RedisReplyCallback cb, int64_t timeout_ms, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    // fixme.
    int status = Dov(cb, timeout_ms, fmt, ap);
    va_end(ap);
    return status;
}

int RedisConnection::Do(RedisReplyCallback cb, int64_t timeout_ms, int argc, const char **argv,
                        const size_t *argvlen) {
    std::vector<size_t> _argvlen;
    if (argvlen == nullptr) {
        for (int i = 0; i < argc; i++)
            _argvlen.push_back(strlen(argv[i]));
        argvlen = _argvlen.data();
    }

    RedisConnection::Request req = NewRequest(cb);
    uint64_t *p_seq = new uint64_t(req.seq);

    int status = redisAsyncCommandArgv(context_, OnReply, p_seq, argc, argv, argvlen);
    if (status != REDIS_OK)
        return NET_ERR;

    // Add timer.
    TimerId timer_id = loop_->RunAfter(
        timeout_ms, std::bind(&RedisConnection::OnTimeout, shared_from_this(), req.seq));
    req.timer_id = timer_id;
    cache_requests_[req.seq] = req;

    return NET_OK;
}

void RedisConnection::Close(bool from_hiredis) {
    loop_->AssertIsCurrent();
    if (closed_)
        return;
    closed_ = true;

    auto context = context_;
    context_ = nullptr;

    if (!from_hiredis) {
        // Use redisAsyncFree() instead of redisAsyncDisconnect(),
        // because redisAsyncDisconnect() don't release context immediately when
        // there are pending replies in redis context.
        redisAsyncFree(context);
    }

    if (disconnect_callback_)
        disconnect_callback_(shared_from_this());
}

void RedisConnection::OnConnect(const redisAsyncContext *context, int status) {
    auto conn = static_cast<RedisConnection *>(context->ev.data);
    RedisConnectionPtr guard(conn->shared_from_this());

    if (conn->connect_callback_)
        conn->connect_callback_(guard, status == REDIS_OK ? NET_OK : NET_ERR);

    // Hiredis connect failed, we should close our RedisConnection.
    if (status != REDIS_OK)
        conn->Close(true);
}

void RedisConnection::OnDisconnect(const redisAsyncContext *context, int status) {
    auto conn = static_cast<RedisConnection *>(context->ev.data);
    RedisConnectionPtr guard(conn->shared_from_this());

    if (conn->disconnect_callback_)
        conn->disconnect_callback_(guard);

    // Hiredis disconnected, we should close our RedisConnection.
    conn->Close(true);
}

void RedisConnection::OnReply(redisAsyncContext *context, void *reply, void *privdata) {
    auto conn = static_cast<RedisConnection *>(context->ev.data);
    RedisConnectionPtr guard(conn->shared_from_this());

    auto p_seq = std::unique_ptr<uint64_t>(static_cast<uint64_t *>(privdata));
    auto iter = conn->cache_requests_.find(*p_seq);
    if (iter == conn->cache_requests_.end()) {
        // Already tiemout.
        return;
    }

    // Cancel timer.
    auto &req = iter->second;
    conn->loop_->CancelTimer(req.timer_id);

    req.callback(static_cast<redisReply *>(reply));
    conn->cache_requests_.erase(iter);
}

void RedisConnection::OnTimeout(RedisConnectionPtr conn, uint64_t seq) {
    auto iter = conn->cache_requests_.find(seq);
    if (iter != conn->cache_requests_.end()) {
        auto req = std::move(iter->second);
        conn->cache_requests_.erase(iter);

        // Invoke callback with timeout error.
        req.callback(nullptr);
    }
}

void RedisConnection::HandleEvents(int events) {
    RedisConnectionPtr guard(shared_from_this());

    // Handle read.
    if (events & Poller::READABLE) {
        loop_->AssertIsCurrent();
        if (context_)
            redisAsyncHandleRead(context_);
    }

    // Handle write.
    if (events & Poller::WRITABLE) {
        loop_->AssertIsCurrent();
        if (context_)
            redisAsyncHandleWrite(context_);
    }
}
