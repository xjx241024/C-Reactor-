#pragma once
#include "TCPServer.h"
#include "EventLoop.h"
#include "Connection.h"
#include "ThreadPool.h"

/*
    BankServer类：网上银行业务类
*/

class UserInfo // 用户(客户端)的信息（状态机）。
{
private:
    int fd_;             // 客户端的fd。
    std::string ip_;     // 客户端的ip地址。
    bool login_ = false; // 客户端登录的状态：true-已登录；false-未登录。
    std::string name_;   // 客户端的用户名。
public:
    UserInfo(int fd, const std::string &ip) : fd_(fd), ip_(ip) {}
    void setLogin(bool login) { login_ = login; }
    bool Login() { return login_; }
};

class BankServer
{
private:
    using spUserInfo = std::shared_ptr<UserInfo>; // 定义一个智能指针类型，指向UserInfo对象。
    TCPServer tcpServer_;
    ThreadPool workThreadPool_; // 工作线程池
    std::map<int, spUserInfo> userInfos_; // 用户状态机
    std::mutex mutex_; // 保护userInfos_的互斥锁

public:
    BankServer(const char *ip, uint16_t port, int subThreadNum = 3, int workThreadNum = 5, uint16_t seq = 1);
    ~BankServer();

    void start(); // 启动服务器，运行事件循环。
    void stop();  // 停止服务器

    void handleMessage(spConnection conn, std::string &message); // 处理客户端的请求报文的回调函数，在TCPServer类中被调用。
    void handleSendComplete(spConnection conn);                  // 数据发送完后，在TCPServer类中回调此函数。
    // void handleEpollTimeout(EventLoop *loop);                  // epoll_wait超时后，在TCPServer中回调此函数
    void handleNewConnection(spConnection conn);   // 处理新客户端连接请求的回调函数，在TCPServer类中被调用。
    void handleCloseConnection(spConnection conn); // 关闭连接，参数是连接对象指针，在TCPServer类中被调用。
    void handleErrorConnection(spConnection conn); // 处理连接错误，参数是连接对象指针，在TCPServer类中被调用。

    void onMessage(spConnection conn, std::string &message); // 处理客户端请求报文，用于添加到工作线程池
    void handleRemove(int fd);  // 客户端的连接超时，在TcpServer类中回调此函数。
};