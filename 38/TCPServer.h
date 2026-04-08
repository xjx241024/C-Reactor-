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
#include <memory>
#include <mutex>

class TCPServer; // 前向声明TCPServer类，避免循环依赖。

// TCP网络服务类
class TCPServer
{
private:
    // 主事件栈内存和堆内存都行。从事件只能堆内存
    EventLoop mainLoop_;                               // 主事件循环
    std::vector<std::unique_ptr<EventLoop>> subLoops_; // 存放从事件的容器
    uint16_t seq_;        // 报文的分隔符
    int threadNum_;                                    // 线程池大小，即从事件循环个数
    ThreadPool threadPool_;                            // 线程池
    Acceptor acceptor_;                                // Acceptor对象，负责接受客户端的连接请求。一个TCPServer对象只有一个Acceptor对象
    std::map<int, spConnection> connections_;          // 一个TCPServer对象可以有多个Connection对象，使用一个map来存储它们
    std::mutex mutex_;                                  // 保护conns_的互斥锁

    std::function<void(spConnection, std::string &message)> onMessageCallback_;
    std::function<void(spConnection)> sendCompleteCallback_;
    std::function<void(spConnection)> closeConnectionCallback_;
    std::function<void(spConnection)> errorConnectionCallback_;
    std::function<void(EventLoop *)> epollTimeoutCallback_;
    std::function<void(spConnection)> newConnectionCallback_;
    std::function<void(int)> timerCallback_;
    std::function<void(int)> removeTimeoutConnCallback_;

public:
    TCPServer(const char *ip, uint16_t port, int threadnum = 3, uint16_t seq = 1); // 构造函数，参数是IP地址和端口号，在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
    ~TCPServer();
    void start(); // 启动服务器，运行事件循环。
    void stop();  // 停止服务器，停止IO线程

    void onMessage(spConnection conn, std::string &message); // 处理客户端的请求报文的回调函数，在Connection类中被调用。
    void sendComplete(spConnection conn);                    // 通知TCPServer：数据发送完后，在Connection类中回调此函数。
    void closeConnection(spConnection conn);                 // 关闭连接，参数是连接对象指针，在Connection类中被调用。
    void errorConnection(spConnection conn);                 // 处理连接错误，参数是连接对象指针，在Connection类中被调用
    void epollTimeout(EventLoop *loop);                      // epoll_wait超时后，在EventLoop中回调此函数
    void newConnection(std::unique_ptr<Socket> clientSock);  // 处理新客户端连接请求的回调函数，在Acceptor类中被调用。
    void removeTimeoutConn(int fd);                          // 删除conns_中的Connection对象，在EventLoop::handletimer()中将回调此函数。

    void setOnMessageCallback(std::function<void(spConnection, std::string &message)> cb);
    void setSendCompleteCallback(std::function<void(spConnection)> cb);
    void setCloseConnectionCallback(std::function<void(spConnection)> cb);
    void setErrorConnectionCallback(std::function<void(spConnection)> cb);
    void setEpollTimeoutCallback(std::function<void(EventLoop *)> cb);
    void setNewConnectionCallback(std::function<void(spConnection)> cb);
    void setRemoveTimeoutConnCallback(std::function<void(int)> cb);
};