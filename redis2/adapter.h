#ifndef _NET_REDIS_ADAPTER_H_
#define _NET_REDIS_ADAPTER_H_

struct redisAsyncContext;
class RedisConnection;

void RedisNetAddRead(void *privdata);
void RedisNetDelRead(void *privdata);
void RedisNetAddWrite(void *privdata);
void RedisNetDelWrite(void *privdata);
void RedisNetCleanUp(void *privdata);

void RedisNetAttach(redisAsyncContext *ac, RedisConnection *conn);

#endif
