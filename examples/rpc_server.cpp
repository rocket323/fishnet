#include "rpc/rpc_server.h"
#include "examples/echo.pb.h"
#include <iostream>

using namespace fishnet;
using namespace echo;

class EchoServiceImpl : public EchoService {
public:
    void Echo(google::protobuf::RpcController* controller,
              const EchoRequest* request,
              EchoResponse* response,
              google::protobuf::Closure* done) override {
        // std::cout << "Received: " << request->message() << std::endl;
        response->set_message("echo: " + request->message());
        done->Run();
    }
};

int main() {
    auto loop = EventLoop::Current();
    InetAddr addr(8888);
    RpcServer server(loop, addr);
    EchoServiceImpl service;
    server.RegisterService(&service);
    if (!server.Start()) {
        std::cerr << "RpcServer start failed" << std::endl;
        return -1;
    }
    std::cout << "RpcServer started at " << addr.IpPort() << std::endl;
    loop->Loop();
    return 0;
}
