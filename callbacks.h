#ifndef _NET_CALLBACKS_H_
#define _NET_CALLBACKS_H_

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;
class InetAddr;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

// For eventor
typedef std::function<void(int)> EventsCallback;

// For acceptor
typedef std::function<void(int fd)> AcceptCallback;

// For tcp connection
typedef std::function<void(TcpConnectionPtr)> ConnectionCallback;
typedef std::function<void(TcpConnectionPtr)> CloseCallback;
typedef std::function<void(TcpConnectionPtr, Buffer *)> ReadCallback;
typedef std::function<void(TcpConnectionPtr)> WriteCompleteCallback;

#endif
