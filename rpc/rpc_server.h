#ifndef _RPC_RPC_SERVER_H_
#define _RPC_RPC_SERVER_H_

#include <google/protobuf/service.h>
#include <map>
#include "rpc/rpc_codec.h"
#include "tcp/tcp_server.h"

namespace fishnet {

class RpcServer {
public:
    RpcServer(EventLoop* loop, const InetAddr& listenAddr);

    void RegisterService(google::protobuf::Service* service);
    bool Start();

private:
    struct ResponseContext {
        TcpConnectionPtr conn;
        uint64_t id;
    };

    void OnConnection(const TcpConnectionPtr& conn);
    void OnRpcMessage(const TcpConnectionPtr& conn, const RpcMessagePtr& message);
    void SendResponse(google::protobuf::Message* response, ResponseContext ctx);

    TcpServer server_;
    RpcCodec codec_;
    std::map<std::string, google::protobuf::Service*> services_;
};

} // namespace fishnet

#endif
