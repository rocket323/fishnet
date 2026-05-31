#include "rpc/rpc_codec.h"
#include <arpa/inet.h>
#include <cstring>
#include "tcp/buffer.h"

namespace fishnet {

void RpcCodec::OnMessage(const TcpConnectionPtr& conn, Buffer* buf) {
    while (buf->ReadableBytes() >= 4) {
        int32_t len = 0;
        memcpy(&len, buf->Peek(), 4);
        len = ntohl(len);

        if (len > 64 * 1024 * 1024 || len < 0) {
            conn->Close();
            break;
        }

        if (buf->ReadableBytes() >= static_cast<size_t>(len + 4)) {
            buf->Retrieve(4);
            RpcMessagePtr message(new RpcMessage);
            if (message->ParseFromArray(buf->Peek(), len)) {
                buf->Retrieve(len);
                message_callback_(conn, message);
            } else {
                conn->Close();
                break;
            }
        } else {
            break;
        }
    }
}

void RpcCodec::Send(const TcpConnectionPtr& conn, const RpcMessage& message) {
    std::string data;
    message.SerializeToString(&data);
    int32_t len = static_cast<int32_t>(data.size());
    int32_t net_len = htonl(len);
    
    char b[4];
    memcpy(b, &net_len, 4);
    conn->Write(b, 4);
    conn->Write(data);
}

} // namespace fishnet
