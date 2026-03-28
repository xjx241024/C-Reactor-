#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include <functional>
#include "Buffer.h"

class Connection
{
private:
    EventLoop *loop_;                                 // Connection对应的事件循环，从构造函数传入，来自TCPServer。
    Socket *connectionSocket_;                        // Connection对应的套接字，负责与客户端进行通信。
    Channel *connectionChannel_;                      // Connection对应的Channel，负责监视连接套接字的读写事件。
    std::function<void(Connection *)> closeCallback_; // 连接关闭(断开)时的回调函数。
    std::function<void(Connection *)> errorCallback_; // 连接发生错误时的回调函数。
    std::function<void(Connection *, const std::string &)> onMessageCallback_; // 处理消息的回调函数，参数是一个无参数、无返回值的函数对象。
    Buffer inputBuffer_;                              // 输入缓冲区
    Buffer outputBuffer_;                             // 输出缓冲区
    // 为什么可以放在栈上？因为Buffer类中没有new操作，Buffer对象的生命周期和Connection对象一样，所以可以放在栈上。

public:
    Connection(EventLoop *loop, Socket *socket); // 构造函数，参数是事件循环对象指针和套接字对象指针，在构造函数中创建一个非阻塞的套接字文件描述符，并用返回值来构造一个Socket对象。
    ~Connection();

    int fd() const;         // 获取连接套接字的文件描述符。
    const char *ip() const; // 获取连接套接字的IP地址字符串。
    uint16_t port() const;  // 获取连接套接字的端口号

    void closeCallback(); // 连接关闭(断开)时的回调函数
    void errorCallback(); // 连接发生错误时的回调函数

    void setCloseCallback(std::function<void(Connection *)> cb); // 设置连接关闭(断开)时的回调函数。
    void setErrorCallback(std::function<void(Connection *)> cb); // 设置连接发生错误时的回调函数。
    void setOnMessageCallback(std::function<void(Connection *, const std::string &)> cb); // 设置处理消息的回调函数，参数是一个无参数、无返回值的函数对象。

    void onMessage(); // 处理消息的函数
};