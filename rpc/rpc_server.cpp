#include "rpc/rpc_server.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/callback.h>

namespace fishnet {

RpcServer::RpcServer(EventLoop* loop, const InetAddr& listenAddr)
    : server_(loop, listenAddr),
      codec_(std::bind(&RpcServer::OnRpcMessage, this, std::placeholders::_1, std::placeholders::_2)) {
    server_.SetConnectionCallback(std::bind(&RpcServer::OnConnection, this, std::placeholders::_1));
    server_.SetReadCallback(std::bind(&RpcCodec::OnMessage, &codec_, std::placeholders::_1, std::placeholders::_2));
}

void RpcServer::RegisterService(google::protobuf::Service* service) {
    const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
    services_[desc->full_name()] = service;
}

bool RpcServer::Start() {
    return server_.Start();
}

void RpcServer::OnConnection(const TcpConnectionPtr& conn) {
}

void RpcServer::OnRpcMessage(const TcpConnectionPtr& conn, const RpcMessagePtr& message) {
    if (message->type() == RpcMessage::REQUEST) {
        auto it = services_.find(message->service());
        if (it != services_.end()) {
            google::protobuf::Service* service = it->second;
            const google::protobuf::ServiceDescriptor* desc = service->GetDescriptor();
            const google::protobuf::MethodDescriptor* method = desc->FindMethodByName(message->method());
            if (method) {
                std::unique_ptr<google::protobuf::Message> request(service->GetRequestPrototype(method).New());
                if (request->ParseFromString(message->payload())) {
                    google::protobuf::Message* response = service->GetResponsePrototype(method).New();
                    ResponseContext ctx = {conn, message->id()};
                    service->CallMethod(method, nullptr, request.get(), response, 
                        google::protobuf::NewCallback<RpcServer, google::protobuf::Message*, ResponseContext>(
                            this, &RpcServer::SendResponse, response, ctx));
                }
            }
        }
    }
}

void RpcServer::SendResponse(google::protobuf::Message* response, ResponseContext ctx) {
    RpcMessage message;
    message.set_type(RpcMessage::RESPONSE);
    message.set_id(ctx.id);
    message.set_payload(response->SerializeAsString());
    codec_.Send(ctx.conn, message);
    delete response;
}

} // namespace fishnet
