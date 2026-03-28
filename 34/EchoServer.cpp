#include "EchoServer.h"

EchoServer::EchoServer(const char *ip, uint16_t port, int subThreadNum, int workThreadNum)
    : tcpServer_(ip, port, subThreadNum), workThreadPool_(workThreadNum, "Work")
{
    // 设置回调函数
    tcpServer_.setNewConnectionCallback(std::bind(&EchoServer::handleNewConnection, this, std::placeholders::_1));
    tcpServer_.setCloseConnectionCallback(std::bind(&EchoServer::handleCloseConnection, this, std::placeholders::_1));
    tcpServer_.setErrorConnectionCallback(std::bind(&EchoServer::handleErrorConnection, this, std::placeholders::_1));
    tcpServer_.setSendCompleteCallback(std::bind(&EchoServer::handleSendComplete, this, std::placeholders::_1));
    // tcpServer_.setEpollTimeoutCallback(std::bind(&EchoServer::handleEpollTimeout, this, std::placeholders::_1));
    tcpServer_.setOnMessageCallback(std::bind(&EchoServer::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
}

EchoServer::~EchoServer()
{
}

void EchoServer::start()
{
    tcpServer_.start();
}

void EchoServer::handleNewConnection(spConnection conn)
{
    std::cout << "EchoServer::handleNewConnection()运行： New Connection Come In." << std::endl;
    // printf("EchoServer::handleNewConnection() thread is %ld.\n",syscall(SYS_gettid));     // 显示线程ID。
    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleCloseConnection(spConnection conn)
{
    std::cout << "EchoServer::handleCloseConnection()运行： EchoServer conn closed." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleErrorConnection(spConnection conn)
{
    std::cout << "EchoServer::handleErrorConnection()运行： EchoServer conn error." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleSendComplete(spConnection conn)
{
    std::cout << "EchoServer::handleSendComplete()运行： Message send complete." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

// void EchoServer::handleEpollTimeout(EventLoop *loop)
// {
//     std::cout << "EchoServer timeout." << std::endl;

//     // 根据业务的需求，在这里可以增加其它的代码。
// }

void EchoServer::handleMessage(spConnection conn, std::string &message)
{
    // printf("EchoServer::handleMessage() thread is %ld.\n",syscall(SYS_gettid));     // 显示线程ID。

    if (workThreadPool_.size() == 0)
    {
        // 如果没有工作线程，在IO线程中计算
        onMessage(conn, message);
    }
    else
    {
        // 有工作线程，则在工作线程计算
        workThreadPool_.addtask(std::bind(&EchoServer::onMessage, this, conn, message)); // 添加到工作线程池
    }
}

void EchoServer::onMessage(spConnection conn, std::string &message)
{
    // 回显业务
    message = "Reply: " + message;              // 构造一个回复报文。
    conn->send(message.data(), message.size()); // sendBuffer = message + len，即报文=报文内容+报文头部
}