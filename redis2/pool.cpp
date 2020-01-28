#include "pool.h"
#include "event_loop.h"

RedisPool::RedisPool(EventLoop *loop, int max_idle, const std::string &ip, int port,
                     const std::string passwd)
    : loop_(loop), max_idle_(max_idle), addr_(ip, port), passwd_(passwd) {
}

RedisPool::~RedisPool() {
    // Close all connection.
    for (auto &conn : idle_conns_)
        conn->Close();

    idle_conns_.clear();
}

RedisConnectionPtr RedisPool::GetConn() {
    RedisConnectionPtr conn;

    while (!idle_conns_.empty()) {
        conn.swap(*idle_conns_.begin());
        idle_conns_.erase(idle_conns_.begin());
        if (!conn->IsClosed())
            break;
    }

    // Fast path: get an idle connection.
    if (conn)
        return conn;

    // Slow path: create a new connection.
    conn = RedisConnection::Connect(loop_, addr_, passwd_);
    if (!conn)
        return conn;

    return conn;
}

void RedisPool::PutConn(RedisConnectionPtr conn) {
    if (!conn->IsClosed() && idle_conns_.size() < max_idle_)
        idle_conns_.push_back(conn);
    else
        conn->Close();
}
