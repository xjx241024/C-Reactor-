#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include <functional>
#include "Connection.h"
#include <memory>

class Acceptor
{
private:
    EventLoop *loop_; // Acceptor对应的事件循环，从构造函数传入，来自TCPServer。
    Socket acceptSocket_; // 监听套接字，负责接受客户端的连接请求,与 Acceptor 的生命周期完全一致
    Channel acceptChannel_; // 监听套接字对应的Channel，负责监视监听套接字的读事件,与 Acceptor 的生命周期完全一致
    std::function<void(std::unique_ptr<Socket>)> newConnectionCallback_; // 处理新连接的回调函数，表示一个参数为Socket*、无返回值的函数对象。

public:
    Acceptor(EventLoop *loop, const char *ip, uint16_t port); // 构造函数，参数是事件循环对象指针、IP地址和端口号，在构造函数中创建一个非阻塞的套接字文件描述符，并用返回值来构造一个Socket对象。
    ~Acceptor();

    void newConnection(); // // 处理新客户端连接请求。

    void listen(); // 监听函数，调用Socket类的listen()函数，让服务器开始监听客户端的连接请求。
    void handleRead(); // 处理读事件的回调函数，调用Channel类的newConnection()函数，接受客户端的连接请求，并将新连接的文件描述符添加到事件循环中。
    void setNewConnectionCallback(std::function<void(std::unique_ptr<Socket>)> cb); // 设置处理新连接的回调函数。
};