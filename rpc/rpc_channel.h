#ifndef _RPC_RPC_CHANNEL_H_
#define _RPC_RPC_CHANNEL_H_

#include <google/protobuf/service.h>
#include <map>
#include <mutex>
#include "rpc/rpc_codec.h"

namespace fishnet {

class RpcChannel : public google::protobuf::RpcChannel {
public:
    RpcChannel(const TcpConnectionPtr& conn);
    ~RpcChannel() override;

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller,
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response,
                    google::protobuf::Closure* done) override;

    void OnRpcMessage(const TcpConnectionPtr& conn, const RpcMessagePtr& message);

private:
    struct PendingCall {
        google::protobuf::Message* response;
        google::protobuf::Closure* done;
    };

    TcpConnectionPtr conn_;
    RpcCodec codec_;
    std::mutex mutex_;
    uint64_t next_id_;
    std::map<uint64_t, PendingCall> pending_calls_;
};

} // namespace fishnet

#endif
