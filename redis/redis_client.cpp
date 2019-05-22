#include "redis_client.h"
#include <assert.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include "event_loop.h"
#include "hiredis.h"
#include "redis_connection.h"
#include "tcp/socket.h"

using namespace std::placeholders;

RedisClient::RedisClient(EventLoop *event_loop, const std::string &domain, int port, const std::string &passwd)
    : event_loop_(event_loop), redis_addrs_(1, IpPort(domain, port)), redis_addr_idx_(0), passwd_(passwd)
{
    assert(!redis_addrs_.empty());
}

RedisClient::RedisClient(EventLoop *event_loop, const InetAddr &addr, const std::string &passwd)
    : event_loop_(event_loop),
      redis_addrs_(1, IpPort(addr.Ip(), addr.HostOrderPort())),
      redis_addr_idx_(0),
      passwd_(passwd)
{
    assert(!redis_addrs_.empty());
}

RedisClient::RedisClient(EventLoop *event_loop, const std::vector<InetAddr> addrs, const std::string &passwd)
    : event_loop_(event_loop), redis_addr_idx_(0), passwd_(passwd)
{
    for (auto &addr : addrs)
        redis_addrs_.push_back(IpPort(addr.Ip(), addr.HostOrderPort()));

    assert(!redis_addrs_.empty());
}

RedisClient::~RedisClient()
{
    HandleClose();
}

int RedisClient::Exec(const RedisReplyCallback &cb, int64_t timeout_ms, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int status = Exec(cb, timeout_ms, fmt, ap);
    va_end(ap);
    return status;
}

int RedisClient::Exec(const RedisReplyCallback &cb, int64_t timeout_ms, const char *fmt, va_list ap)
{
    RedisConnectionPtr conn = GetConn();
    if (!conn)
        return NET_ERR;

    int status = conn->Exec(std::bind(&RedisClient::OnRedisReply, this, cb, conn, _1), fmt, ap);
    if (status != NET_OK)
    {
        conn->Close();
        return status;
    }

    // Add timer
    TimerId timer_id = event_loop_->RunAfter(timeout_ms, std::bind(&RedisClient::OnRedisReplyTimeout, this, conn));
    timer_ids_[conn->Id()] = timer_id;

    return NET_OK;
}

int RedisClient::Exec(const RedisReplyCallback &cb, int64_t timeout_ms, int argc, const char **argv,
                      const size_t *argvlen)
{
    RedisConnectionPtr conn = GetConn();
    if (!conn)
        return NET_ERR;

    int status = conn->Exec(std::bind(&RedisClient::OnRedisReply, this, cb, conn, _1), argc, argv, argvlen);
    if (status != NET_OK)
    {
        conn->Close();
        return status;
    }

    // Add timer
    TimerId timer_id = event_loop_->RunAfter(timeout_ms, std::bind(&RedisClient::OnRedisReplyTimeout, this, conn));
    timer_ids_[conn->Id()] = timer_id;

    return NET_OK;
}

int RedisClient::Exec(const RedisReplyCallback &cb, int64_t timeout_ms, const RedisCommand cmd)
{
    const std::vector<std::string> &args = cmd.args_;
    std::vector<const char *> argv;
    std::vector<size_t> argvlen;
    for (auto iter = args.begin(); iter != args.end(); iter++)
    {
        argv.push_back(iter->c_str());
        argvlen.push_back(iter->length());
    }
    return Exec(cb, timeout_ms, (int)argv.size(), argv.data(), argvlen.data());
}

int RedisClient::MultiExec(const RedisReplyCallback &cb, int64_t timeout_ms, const RedisCommands cmds)
{
    auto conn = GetConn();
    if (!conn)
        return NET_ERR;

    // MULTI
    int status = NET_OK;
    status = conn->Exec(std::bind(&RedisClient::OnStatusReply, this, conn, "OK", _1), "MULTI");
    if (status != NET_OK)
    {
        conn->Close();
        return status;
    }

    // CMDS
    std::vector<const char *> argv;
    std::vector<size_t> argvlen;
    for (auto iter = cmds.cmds_.begin(); iter != cmds.cmds_.end(); iter++)
    {
        const std::vector<std::string> &args = iter->args_;
        argv.resize(args.size());
        argvlen.resize(args.size());
        for (size_t i = 0; i < args.size(); i++)
        {
            argv[i] = args[i].c_str();
            argvlen[i] = args[i].length();
        }
        status = conn->Exec(std::bind(&RedisClient::OnStatusReply, this, conn, "QUEUED", _1), (int)argv.size(),
                            argv.data(), argvlen.data());
        if (status != NET_OK)
        {
            conn->Close();
            return status;
        }
    }

    // EXEC
    status = conn->Exec(std::bind(&RedisClient::OnRedisReply, this, cb, conn, _1), "EXEC");
    if (status != NET_OK)
    {
        conn->Close();
        return status;
    }

    // Add timer
    TimerId timer_id = event_loop_->RunAfter(timeout_ms, std::bind(&RedisClient::OnRedisReplyTimeout, this, conn));
    timer_ids_[conn->Id()] = timer_id;

    return NET_OK;
}

InetAddr RedisClient::GetNextAddr()
{
    size_t idx = redis_addr_idx_;
    if (++redis_addr_idx_ >= redis_addrs_.size())
        redis_addr_idx_ = 0;
    auto &ip_port = redis_addrs_[idx];

    std::string ip = ip_port.ip;
    if (!std::isdigit(ip[0]))
    {
        struct hostent *h;
        h = gethostbyname(ip.c_str());
        if (h == nullptr)
            return -1;

        if (h->h_addr_list == nullptr || *(h->h_addr_list) == nullptr)
            return -1;

        char buf[61];
        inet_ntop(h->h_addrtype, *h->h_addr_list, buf, sizeof(buf));
        ip = std::string(buf);
    }

    return InetAddr(ip, ip_port.port);
}

RedisConnectionPtr RedisClient::GetConn()
{
    auto addr = GetNextAddr();
    RedisConnectionPtr conn;
    auto &idle_list = idle_conns_[addr.IpPort()];

    while (!idle_list.empty())
    {
        conn.swap(idle_list.begin()->second);
        idle_list.erase(idle_list.begin());
        if (!conn->IdleExpired(2 * 60 * 1000))
            break;

        // Close expired connection
        conn->Close();
        conn.reset();
    }
    if (idle_list.empty())
        idle_conns_.erase(addr.IpPort());

    // Fase path: get an idle connection
    if (conn)
        return conn;

    // Slow path: create a new connection
    conn = RedisConnection::Connect(event_loop_, addr);
    if (!conn)
        return conn;

    conn->SetDisconnectCallback(std::bind(&RedisClient::OnDisconnect, this, _1));

    if (passwd_.length() > 0)
    {
        int ret = conn->Exec(std::bind(&RedisClient::OnStatusReply, this, conn, "OK", _1), "AUTH %s", passwd_.c_str());
        if (ret != NET_OK)
        {
            conn->Close();
            conn.reset();
        }
    }

    return conn;
}

void RedisClient::PutConn(RedisConnectionPtr conn)
{
    auto &idle_list = idle_conns_[conn->PeerAddr().IpPort()];

    // At most 16 idle connections per (ip, port)
    if (idle_list.size() < 16)
        idle_list.insert(std::make_pair(conn->Id(), std::move(conn)));
    else
        conn->Close();
}

void RedisClient::OnRedisReply(RedisReplyCallback cb, RedisConnectionPtr conn, redisReply *reply)
{
    // Remove timer
    auto iter = timer_ids_.find(conn->Id());
    if (iter != timer_ids_.end())
    {
        event_loop_->CancelTimer(iter->second);
        timer_ids_.erase(iter);
    }
    cb(reply);

    if (conn->Closed())
        return;

    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        conn->Close();
        return;
    }

    // Put connection into idle list
    PutConn(conn);
}

void RedisClient::OnRedisReplyTimeout(RedisConnectionPtr conn)
{
    timer_ids_.erase(conn->Id());
    conn->Close();
}

void RedisClient::OnDisconnect(RedisConnectionPtr conn)
{
    auto iter = idle_conns_.find(conn->PeerAddr().IpPort());
    if (iter == idle_conns_.end())
        return;

    iter->second.erase(conn->Id());
    if (iter->second.empty())
        idle_conns_.erase(iter);
}

// Expect to receive a reply with specific status("OK" or "QUEUED")
void RedisClient::OnStatusReply(RedisConnectionPtr conn, const char *status, redisReply *reply)
{
    if (reply == nullptr)
    {
        conn->Close();
        return;
    }

    if (reply->type == REDIS_REPLY_ERROR)
    {
        conn->Close();
        return;
    }

    if (reply->type != REDIS_REPLY_STATUS || strcmp(reply->str, status) != 0)
    {
        conn->Close();
        return;
    }
}

void RedisClient::HandleClose()
{
    // Close all connections
    auto idle_conns = std::move(idle_conns_);
    for (auto iter = idle_conns.begin(); iter != idle_conns.end(); iter++)
    {
        for (auto conn_it = iter->second.begin(); conn_it != iter->second.end(); conn_it++)
        {
            conn_it->second->Close();
        }
    }
}
