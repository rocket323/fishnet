#ifndef _RPC_RPC_CODEC_H_
#define _RPC_RPC_CODEC_H_

#include <functional>
#include <memory>
#include "rpc.pb.h"
#include "tcp/tcp_connection.h"

namespace fishnet {

typedef std::shared_ptr<RpcMessage> RpcMessagePtr;

class RpcCodec {
public:
    typedef std::function<void(const TcpConnectionPtr&, const RpcMessagePtr&)> RpcMessageCallback;

    explicit RpcCodec(const RpcMessageCallback& cb)
        : message_callback_(cb) {}

    void OnMessage(const TcpConnectionPtr& conn, Buffer* buf);
    void Send(const TcpConnectionPtr& conn, const RpcMessage& message);

private:
    RpcMessageCallback message_callback_;
};

} // namespace fishnet

#endif
