#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Acceptor.h"

class TCPServer; // 前向声明TCPServer类，避免循环依赖。

// TCP网络服务类
class TCPServer
{
private:
    // 在栈上创建一个EventLoop对象，生命周期与TCPServer对象相同
    EventLoop loop_; // 事件循环对象，当TCPServer结束时，事件循环对象也应当被销毁，所以在栈上创建
    //EventLoop *loop_; // 事件循环对象指针，TCPServer对象中有一个EventLoop对象指针成员。
    Acceptor *acceptor_; // Acceptor对象，负责接受客户端的连接请求。
public:
    TCPServer(const char *ip, uint16_t port); // 构造函数，参数是IP地址和端口号，在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
    ~TCPServer();
    void start(); // 启动服务器，运行事件循环。
};