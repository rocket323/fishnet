#ifndef _REDIS_POOL_H_
#define _REDIS_POOL_H_

#include <map>
#include "conn.h"
#include "util.h"

class RedisPool {
public:
    RedisPool(int max_idle, const std::string &ip = "127.0.0.1", int port = 6379,
              const std::string passwd = "");
    ~RedisPool();

    RedisConnectionPtr GetConn();
    void PutConn();

private:
    void OnDisconnect(RedisConnectionPtr conn);

private:
    int max_idle_;
    const std::string ip_;
    const int port_;
    const std::string passwd_;
    std::map<uint64_t, RedisConnectionPtr> idle_conns_;
};

#endif
