#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Connection.h"
#include <map>

class TCPServer; // 前向声明TCPServer类，避免循环依赖。

// TCP网络服务类
class TCPServer
{
private:
    // 在栈上创建一个EventLoop对象，生命周期与TCPServer对象相同。当TCPServer结束时，事件循环对象也应当被销毁，所以在栈上创建
    EventLoop loop_;
    Acceptor *acceptor_; // Acceptor对象，负责接受客户端的连接请求。一个TCPServer对象只有一个Acceptor对象
    std::map<int, Connection*> connections_; // 一个TCPServer对象可以有多个Connection对象，使用一个map来存储它们

public:
    TCPServer(const char *ip, uint16_t port); // 构造函数，参数是IP地址和端口号，在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
    ~TCPServer();
    void start(); // 启动服务器，运行事件循环。

    void newConnection(Socket *clientSock); // 处理新客户端连接请求的回调函数，在Acceptor类中被调用。
    void closeConnection(Connection *conn); // 关闭连接，参数是连接对象指针，在Connection类中被调用。
    void errorConnection(Connection *conn); // 处理连接错误，参数是连接对象指针，在Connection类中被调用。
    // 备注，传入Connection对象指针的原因有二：
    // 1. 在Connection类中，调用TCPServer类的closeConnection()函数和errorConnection()函数时，无法直接传入连接套接字的文件描述符，因为TCPServer类的connections_ map是以连接套接字的文件描述符为键，以Connection对象指针为值的，所以需要传入Connection对象指针来获取连接套接字的文件描述符。
    // 2. 在TCPServer类的closeConnection()函数和errorConnection()函数中，需要调用Connection对象的fd()函数来获取连接套接字的文件描述符，以便从connections_ map中删除对应的Connection对象指针，所以需要传入Connection对象指针来调用fd()函数。
};