#pragma once
#include "TCPServer.h"
#include "EventLoop.h"
#include "Connection.h"

class EchoServer
{
private:
    TCPServer tcpServer_;

public:
    EchoServer(const char *ip, uint16_t port, int threadNum_ = 3);
    ~EchoServer();

    void start(); // 启动服务器，运行事件循环。

    void handleMessage(Connection *conn, std::string &message); // 处理客户端的请求报文的回调函数，在TCPServer类中被调用。
    void handleSendComplete(Connection *conn);                 // 数据发送完后，在TCPServer类中回调此函数。
    // void handleEpollTimeout(EventLoop *loop);                  // epoll_wait超时后，在TCPServer中回调此函数
    void handleNewConnection(Connection *conn);                // 处理新客户端连接请求的回调函数，在TCPServer类中被调用。
    void handleCloseConnection(Connection *conn);              // 关闭连接，参数是连接对象指针，在TCPServer类中被调用。
    void handleErrorConnection(Connection *conn);              // 处理连接错误，参数是连接对象指针，在TCPServer类中被调用。
};