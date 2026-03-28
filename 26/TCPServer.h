#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "Connection.h"
#include "ThreadPool.h"
#include <map>
#include "functional"

class TCPServer; // 前向声明TCPServer类，避免循环依赖。

// TCP网络服务类
class TCPServer
{
private:
    // 主事件栈内存和堆内存都行。从事件只能堆内存
    EventLoop mainLoop_;    // 主事件循环
    std::vector<EventLoop *> subLoops_; // 存放从事件的容器
    ThreadPool *threadPool_; // 线程池
    int threadNum_; // 线程池大小，即从事件循环个数
    Acceptor *acceptor_; // Acceptor对象，负责接受客户端的连接请求。一个TCPServer对象只有一个Acceptor对象
    std::map<int, Connection*> connections_; // 一个TCPServer对象可以有多个Connection对象，使用一个map来存储它们
    
    std::function<void(Connection*, std::string &message)> onMessageCallback_;
    std::function<void(Connection*)> sendCompleteCallback_;
    std::function<void(Connection*)> closeConnectionCallback_;
    std::function<void(Connection*)> errorConnectionCallback_;
    std::function<void(EventLoop*)> epollTimeoutCallback_;
    std::function<void(Connection*)> newConnectionCallback_;

public:
    TCPServer(const char *ip, uint16_t port, int threadnum = 3); // 构造函数，参数是IP地址和端口号，在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
    ~TCPServer();
    void start(); // 启动服务器，运行事件循环。

    void onMessage(Connection *conn, std::string &message); // 处理客户端的请求报文的回调函数，在Connection类中被调用。
    void sendComplete(Connection *conn);    // 通知TCPServer：数据发送完后，在Connection类中回调此函数。
    void closeConnection(Connection *conn); // 关闭连接，参数是连接对象指针，在Connection类中被调用。
    void errorConnection(Connection *conn); // 处理连接错误，参数是连接对象指针，在Connection类中被调用
    void epollTimeout(EventLoop *loop);     // epoll_wait超时后，在EventLoop中回调此函数
    void newConnection(Socket *clientSock); // 处理新客户端连接请求的回调函数，在Acceptor类中被调用。

    void setOnMessageCallback(std::function<void(Connection*, std::string &message)> cb);
    void setSendCompleteCallback(std::function<void(Connection*)> cb);
    void setCloseConnectionCallback(std::function<void(Connection*)> cb);
    void setErrorConnectionCallback(std::function<void(Connection*)> cb);
    void setEpollTimeoutCallback(std::function<void(EventLoop*)> cb);
    void setNewConnectionCallback(std::function<void(Connection*)> cb);


};