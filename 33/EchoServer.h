#pragma once
#include "TCPServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"

class EchoServer
{
private:
    TCPServer tcpServer_;
    ThreadPool workThreadPool_; // 工作线程池

public:
    EchoServer(const char *ip, uint16_t port, int subThreadNum = 3, int workThreadNum = 5);
    ~EchoServer();

    void start(); // 启动服务器，运行事件循环。

    void handleMessage(spConnection conn, std::string &message); // 处理客户端的请求报文的回调函数，在TCPServer类中被调用。
    void handleSendComplete(spConnection conn);                 // 数据发送完后，在TCPServer类中回调此函数。
    // void handleEpollTimeout(EventLoop *loop);                  // epoll_wait超时后，在TCPServer中回调此函数
    void handleNewConnection(spConnection conn);                // 处理新客户端连接请求的回调函数，在TCPServer类中被调用。
    void handleCloseConnection(spConnection conn);              // 关闭连接，参数是连接对象指针，在TCPServer类中被调用。
    void handleErrorConnection(spConnection conn);              // 处理连接错误，参数是连接对象指针，在TCPServer类中被调用。

    void onMessage(spConnection conn, std::string &message);  // 处理客户端请求报文，用于添加到工作线程池

};