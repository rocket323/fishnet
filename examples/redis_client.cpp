// #include "redis/redis_client.h"
#include <cstring>
#include "redis2/conn.h"
#include "redis2/pool.h"

using namespace std::placeholders;

void OnReply(redisReply *reply) {
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
        printf("error: %s\n", reply ? reply->str : "nil reply");
        EventLoop::Current()->Stop();
        return;
    }
    if (reply->type == REDIS_REPLY_NIL)
        puts("<nil>");
    else if (reply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < reply->elements; i++) {
            printf("%d) ", i);
            OnReply(reply->element[i]);
        }
    } else if (reply->type == REDIS_REPLY_INTEGER)
        printf("%lld\n", reply->integer);
    else
        printf("%s\n", reply->str);
    EventLoop::Current()->Stop();
}

int main(int argc, char **argv) {
    auto loop = EventLoop::Current();
    InetAddr addr(6379);

    if (argc < 2) {
        puts("argc should be >= 2");
        return -1;
    }

    RedisPool pool(loop, 16);
    auto conn = pool.GetConn();
    if (!conn) {
        puts("connect failed!");
        return -1;
    }
    conn->Do(OnReply, 1000, argc - 1, (const char **)(argv + 1));

    loop->Loop();
    return 0;
}
