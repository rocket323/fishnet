#include "rpc/rpc_channel.h"
#include "tcp/connector.h"
#include "examples/echo.pb.h"
#include <iostream>

using namespace fishnet;
using namespace echo;

void OnReply(EchoResponse* response) {
    std::cout << "Reply: " << response->message() << std::endl;
    EventLoop::Current()->Stop();
}

int main() {
    auto loop = EventLoop::Current();
    InetAddr addr("127.0.0.1", 8888);
    
    auto conn = Connector::Connect(loop, addr);
    if (!conn) {
        std::cerr << "Connect failed" << std::endl;
        return -1;
    }

    RpcChannel channel(conn);
    EchoService::Stub stub(&channel);

    EchoRequest request;
    request.set_message("hello fishnet rpc");
    EchoResponse* response = new EchoResponse;

    stub.Echo(nullptr, &request, response, google::protobuf::NewCallback(OnReply, response));

    loop->Loop();
    delete response;
    return 0;
}
