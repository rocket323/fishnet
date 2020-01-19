#include "redis_adapter.h"
#include <assert.h>
#include "async.h"
#include "redis_connection.h"

void RedisNetAddRead(void *privdata) {
    assert(privdata != nullptr);
    RedisConnection *conn = static_cast<RedisConnection *>(privdata);
    conn->EnableReading();
}

void RedisNetDelRead(void *privdata) {
    assert(privdata != nullptr);
    RedisConnection *conn = static_cast<RedisConnection *>(privdata);
    conn->DisableReading();
}

void RedisNetAddWrite(void *privdata) {
    assert(privdata != nullptr);
    RedisConnection *conn = static_cast<RedisConnection *>(privdata);
    conn->EnableWriting();
}

void RedisNetDelWrite(void *privdata) {
    assert(privdata != nullptr);
    RedisConnection *conn = static_cast<RedisConnection *>(privdata);
    conn->DisableWriting();
}

void RedisNetCleanUp(void *privdata) {
    assert(privdata != nullptr);
    RedisConnection *conn = static_cast<RedisConnection *>(privdata);
    conn->Remove();
}

void RedisNetAttach(redisAsyncContext *ac, RedisConnection *conn) {
    ac->ev.addRead = RedisNetAddRead;
    ac->ev.delRead = RedisNetDelRead;
    ac->ev.addWrite = RedisNetAddWrite;
    ac->ev.delWrite = RedisNetDelWrite;
    ac->ev.data = conn;
}
