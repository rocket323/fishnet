#ifndef _NET_REDIS_CLIENT_H_
#define _NET_REDIS_CLIENT_H_

#include <map>
#include <vector>
#include "async.h"
#include "event_loop.h"
#include "redis_command.h"
#include "redis_connection.h"
#include "tcp/inet_addr.h"

class RedisClient {
public:
    RedisClient(EventLoop *event_loop, const std::string &domain, int port,
                const std::string &passwd = "");
    RedisClient(EventLoop *event_loop, const InetAddr &addr, const std::string &passwd = "");
    RedisClient(EventLoop *event_loop, const std::vector<InetAddr> addrs,
                const std::string &passwd = "");
    ~RedisClient();

    int Do(const RedisReplyCallback &cb, int64_t timeout_ms, const char *fmt, ...);
    int Do(const RedisReplyCallback &cb, int64_t timeout_ms, const char *fmt, va_list ap);
    int Do(const RedisReplyCallback &cb, int64_t timeout_ms, int argc, const char **argv,
           const size_t *argvlen = nullptr);
    int Do(const RedisReplyCallback &cb, int64_t timeout_ms, const RedisCommand cmd);
    int DoMulti(const RedisReplyCallback &cb, int64_t timeout_ms, const RedisCommands cmds);

private:
    InetAddr GetNextAddr();
    RedisConnectionPtr GetConn();
    void PutConn(RedisConnectionPtr conn);

    void OnRedisReply(RedisReplyCallback cb, RedisConnectionPtr conn, redisReply *reply);
    void OnRedisReplyTimeout(RedisConnectionPtr conn);
    void OnDisconnect(RedisConnectionPtr conn);

    // Expect to receive a reply with specific status("OK" or "QUEUED")
    void OnStatusReply(RedisConnectionPtr conn, const char *status, redisReply *reply);

    void HandleClose();

private:
    EventLoop *event_loop_;
    std::map<std::string, std::map<uint64_t, RedisConnectionPtr>> idle_conns_;
    std::map<uint64_t, TimerId> timer_ids_;

    struct IpPort {
        std::string ip;
        int port;
        IpPort(const std::string &ip_, int port_) : ip(ip_), port(port_) {}
    };

    // Redis server address
    std::vector<IpPort> redis_addrs_;
    size_t redis_addr_idx_;

    const std::string passwd_;
};

#endif
