#include "redis/redis_client.h"
#include <cstring>

using namespace std::placeholders;

void OnReply2(redisReply *reply, RedisClient *client, int idx) {
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
        printf("error: %s\n", reply ? reply->str : "nil reply");
        EventLoop::Current()->Stop();
        return;
    }
    printf("%s\n", reply->str);

    if (idx < 10)
        client->Do(std::bind(&OnReply2, _1, client, idx + 1), 1000, "PING");
    else
        EventLoop::Current()->Stop();
}

void OnReply(redisReply *reply) {
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR) {
        printf("error: %s\n", reply ? reply->str : "nil reply");
        EventLoop::Current()->Stop();
        return;
    }
    if (reply->type == REDIS_REPLY_NIL)
        puts("<nil>");
    else if (reply->type == REDIS_REPLY_ARRAY)
        puts("<array>");
    else if (reply->type == REDIS_REPLY_INTEGER)
        printf("%lld\n", reply->integer);
    else
        printf("%s\n", reply->str);
    EventLoop::Current()->Stop();
}

int main(int argc, char **argv) {
    auto loop = EventLoop::Current();
    RedisClient client(loop, InetAddr(6379));

    if (argc > 1)
        client.Do(OnReply, 1000, argc - 1, (const char **)(argv + 1));
    else
        client.Do(std::bind(&OnReply2, _1, &client, 0), 1000, "PING");

    loop->Loop();
    return 0;
}
