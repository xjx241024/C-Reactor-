#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include <functional>

class Connection
{
private:
    EventLoop *loop_; // Connection对应的事件循环，从构造函数传入，来自TCPServer。
    Socket *connectionSocket_; // Connection对应的套接字，负责与客户端进行通信。
    Channel *connectionChannel_; // Connection对应的Channel，负责监视连接套接字的读写事件。
    std::function<void(Connection *)> closeCallback_; // 连接关闭(断开)时的回调函数，类型为std::function<void(Connection*)>，表示一个参数为Connection对象指针、无返回值的函数对象。
    std::function<void(Connection *)> errorCallback_; // 连接发生错误时的回

public:
    Connection(EventLoop *loop, Socket *socket); // 构造函数，参数是事件循环对象指针和套接字对象指针，在构造函数中创建一个非阻塞的套接字文件描述符，并用返回值来构造一个Socket对象。
    ~Connection();

    int fd() const; // 获取连接套接字的文件描述符。
    const char* ip() const; // 获取连接套接字的IP地址字符串。
    uint16_t port() const;  // 获取连接套接字的端口号

    void closeCallback(); // 连接关闭(断开)时的回调函数
    void errorCallback(); // 连接发生错误时的回调函数

    void setCloseCallback(std::function<void(Connection *)> cb); // 设置连接关闭(断开)时的回调函数。
    void setErrorCallback(std::function<void(Connection *)> cb); // 设置连接发生错误时的回调函数。
};