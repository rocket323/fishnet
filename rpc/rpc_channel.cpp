#include "rpc/rpc_channel.h"
#include <google/protobuf/descriptor.h>

namespace fishnet {

RpcChannel::RpcChannel(const TcpConnectionPtr& conn)
    : conn_(conn),
      codec_(std::bind(&RpcChannel::OnRpcMessage, this, std::placeholders::_1, std::placeholders::_2)),
      next_id_(1) {
    conn_->SetReadCallback(std::bind(&RpcCodec::OnMessage, &codec_, std::placeholders::_1, std::placeholders::_2));
}

RpcChannel::~RpcChannel() {
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
    RpcMessage message;
    message.set_type(RpcMessage::REQUEST);
    uint64_t id = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        id = next_id_++;
        pending_calls_[id] = {response, done};
    }
    message.set_id(id);
    message.set_service(method->service()->full_name());
    message.set_method(method->name());
    message.set_payload(request->SerializeAsString());
    codec_.Send(conn_, message);
}

void RpcChannel::OnRpcMessage(const TcpConnectionPtr& conn, const RpcMessagePtr& message) {
    if (message->type() == RpcMessage::RESPONSE) {
        uint64_t id = message->id();
        PendingCall call = {nullptr, nullptr};
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = pending_calls_.find(id);
            if (it != pending_calls_.end()) {
                call = it->second;
                pending_calls_.erase(it);
            }
        }
        if (call.response) {
            call.response->ParseFromString(message->payload());
            if (call.done) {
                call.done->Run();
            }
        }
    }
}

} // namespace fishnet
