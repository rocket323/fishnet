#ifndef _REDIS_POOL_H_
#define _REDIS_POOL_H_

#include <list>
#include <map>
#include "conn.h"
#include "tcp/inet_addr.h"
#include "util.h"

class EventLoop;

class RedisPool {
public:
    RedisPool(EventLoop *loop, int max_idle, const std::string &ip = "127.0.0.1", int port = 6379,
              const std::string passwd = "");
    ~RedisPool();

    RedisConnectionPtr GetConn();
    void PutConn(RedisConnectionPtr conn);

private:
    EventLoop *loop_;
    int max_idle_;
    InetAddr addr_;
    const std::string passwd_;
    std::list<RedisConnectionPtr> idle_conns_;
};

#endif
