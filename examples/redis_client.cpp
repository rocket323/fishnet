#include "redis/redis_client.h"

using namespace std::placeholders;

void OnReply(redisReply *reply, RedisClient *client, int idx)
{
    if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
    {
        printf("redis reply error\n");
        return;
    }
    printf("%s\n", reply->str);

    if (idx < 10)
        client->Exec(std::bind(&OnReply, _1, client, idx + 1), 1000, "PING");
    else
        EventLoop::Current()->Stop();
}

int main()
{
    auto loop = EventLoop::Current();
    RedisClient client(loop, InetAddr(6379));

    int ret = client.Exec(std::bind(&OnReply, _1, &client, 0), 1000, "PING");

    loop->Loop();
    return 0;
}
