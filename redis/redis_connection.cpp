#include "redis_connection.h"
#include <assert.h>
#include "async.h"
#include "event_loop.h"
#include "hiredis.h"
#include "redis_adapter.h"
#include "tcp/socket.h"
#include "util.h"

using namespace std::placeholders;
uint64_t RedisConnection::next_conn_id_(1);

RedisConnection::RedisConnection(EventLoop *event_loop, redisAsyncContext *context, const InetAddr &local_addr,
                                 const InetAddr &peer_addr)
    : event_loop_(event_loop),
      conn_id_(__sync_fetch_and_add(&next_conn_id_, 1)),
      context_(context),
      eventor_(new Eventor(event_loop, context->c.fd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr),
      closed_(false)
{
    eventor_->SetEventsCallback(std::bind(&RedisConnection::HandleEvents, this, _1));
}

RedisConnection::~RedisConnection()
{
    // Help find memory leaks
    assert(context_ == NULL);
}

int RedisConnection::Execv(const RedisReplyCallback &cb, const char *fmt, va_list ap)
{
    active_ts_ = Util::CurrentSystemTimeMillis();
    int status = redisAsyncCommand(context_, OnRedisReply, NULL, fmt, ap);
    if (status != REDIS_OK)
        return NET_ERR;

    reply_callbacks_.push_back(cb);
    return NET_OK;
}

int RedisConnection::Exec(const RedisReplyCallback &cb, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int status = Execv(cb, fmt, ap);
    va_end(ap);
    return status;
}

int RedisConnection::Exec(const RedisReplyCallback &cb, int argc, const char **argv, const size_t *argvlen)
{
    active_ts_ = Util::CurrentSystemTimeMillis();
    int status = redisAsyncCommandArgv(context_, OnRedisReply, NULL, argc, argv, argvlen);
    if (status != REDIS_OK)
        return NET_ERR;

    reply_callbacks_.push_back(cb);
    return NET_OK;
}

void RedisConnection::OnRedisReply(redisAsyncContext *context, void *reply, void *privdata)
{
    auto conn = static_cast<RedisConnection *>(context->ev.data);
    assert(!conn->reply_callbacks_.empty());
    auto callback = std::move(conn->reply_callbacks_.front());
    conn->reply_callbacks_.pop_front();
    callback((redisReply *)reply);
}

RedisConnectionPtr RedisConnection::Connect(EventLoop *event_loop, const InetAddr &addr)
{
    RedisConnectionPtr conn;
    redisAsyncContext *context = redisAsyncConnect(addr.Ip().c_str(), addr.HostOrderPort());
    if (context == nullptr)
        return conn;

    if (context->err)
    {
        redisAsyncFree(context);
        return conn;
    }

    int sockfd = context->c.fd;
    conn = std::make_shared<RedisConnection>(event_loop, context, sockets::GetLocalAddr(sockfd), addr);

    // Init connection
    RedisNetAttach(context, conn.get());
    redisAsyncSetConnectCallback(context, OnConnect);
    redisAsyncSetDisconnectCallback(context, OnDisconnect);
    conn->EnableReading();
    conn->EnableWriting();

    return conn;
}

void RedisConnection::OnConnect(const redisAsyncContext *context, int status)
{
    auto conn = static_cast<RedisConnection *>(context->ev.data);
    RedisConnectionPtr guard(conn->shared_from_this());

    if (conn->connect_callback_)
        conn->connect_callback_(guard, status == REDIS_OK ? NET_OK : NET_ERR);

    // We know redisAsyncContext is freeing,
    // set context_ to NULL and close wrap RedisConnection.
    if (status != REDIS_OK)
        conn->HandleClose(true);
}

void RedisConnection::OnDisconnect(const redisAsyncContext *context, int status)
{
    auto conn = static_cast<RedisConnection *>(context->ev.data);
    conn->HandleClose(true);
}

void RedisConnection::HandleEvents(int revents)
{
    // Prevent connection beging destroyed in HandleXXX().
    RedisConnectionPtr guard(shared_from_this());

    if (revents & Poller::POLLERR)
        HandleError();

    if ((revents & Poller::POLLHUB) & (revents & ~Poller::POLLIN))
        HandleClose();

    if (revents & Poller::POLLIN)
        HandleRead();

    if (revents & Poller::POLLOUT)
        HandleWrite();
}

void RedisConnection::HandleRead()
{
    event_loop_->AssertIsCurrent();
    if (context_)
        redisAsyncHandleRead(context_);
}

void RedisConnection::HandleWrite()
{
    event_loop_->AssertIsCurrent();
    if (context_)
        redisAsyncHandleWrite(context_);
}

void RedisConnection::HandleError()
{
    event_loop_->AssertIsCurrent();
    HandleClose();
}

// We need to know the reason of closing RedisConnection,
// if it's closed from a hiredis callback,
// we don't free `context_` because hiredis would free it when the callback return.
void RedisConnection::HandleClose(bool from_callback)
{
    event_loop_->AssertIsCurrent();
    if (Closed())
        return;
    closed_ = true;

    auto context = context_;
    context_ = nullptr;

    if (!from_callback)
    {
        // Use redisAsyncFree instead of redisAsyncDisconnect()
        // because redisAsyncDisconnect() don't release context immediately
        // when there are pending replys in redis context.
        redisAsyncFree(context);
    }

    if (disconnect_callback_)
        disconnect_callback_(shared_from_this());
}
