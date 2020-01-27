#include "pool.h"

RedisPool::RedisPool(int max_idle, const std::string &ip, int port, const std::string passwd)
    : max_idle_(max_idle), ip_(ip), port_(port), passwd_(passwd) {
}

RedisPool::~RedisPool() {
}

RedisConnectionPtr RedisPool::GetConn() {
    RedisConnectionPtr conn;

    return conn;
}

void RedisPool::PutConn() {
}
