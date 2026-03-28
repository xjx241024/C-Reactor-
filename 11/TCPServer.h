#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"

class TCPServer; // 前向声明TCPServer类，避免循环依赖。

class TCPServer
{
private:
    EventLoop loop_; // 事件循环对象，TCPServer对象中有一个EventLoop对象成员。
    // 在栈上创建一个EventLoop对象，生命周期与TCPServer对象相同，不需要动态分配内存，也不需要在析构函数中释放内存。
    //EventLoop *loop_; // 事件循环对象指针，TCPServer对象中有一个EventLoop对象指针成员。
public:
    TCPServer(const char *ip, uint16_t port); // 构造函数，参数是IP地址和端口号，在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
    ~TCPServer();
    void start(); // 启动服务器，运行事件循环。
};