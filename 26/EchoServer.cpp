#include "EchoServer.h"

EchoServer::EchoServer(const char *ip, uint16_t port, int threadNum_ = 3):tcpServer_(ip, port, threadNum_)
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

void EchoServer::handleNewConnection(Connection *conn)
{
    std::cout << "New Connection Come In" << std::endl;
    printf("EchoServer::handleNewConnection() thread is %ld.\n",syscall(SYS_gettid));     // 显示线程ID。
    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleCloseConnection(Connection *conn)
{
    std::cout << "EchoServer conn closed." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleErrorConnection(Connection *conn)
{
    std::cout << "EchoServer conn error." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleSendComplete(Connection *conn)
{
    std::cout << "Message send complete." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

// void EchoServer::handleEpollTimeout(EventLoop *loop)
// {
//     std::cout << "EchoServer timeout." << std::endl;

//     // 根据业务的需求，在这里可以增加其它的代码。
// }

void EchoServer::handleMessage(Connection *conn, std::string &message)
{
    printf("EchoServer::handleMessage() thread is %ld.\n",syscall(SYS_gettid));     // 显示线程ID。
    // 回显业务
    message = "Reply: " + message; // 构造一个回复报文。
    // int len = message.size();
    // std::string sendBuffer((char *)&len, 4); // 构造一个临时发送缓冲区，前4个字节存放报文内容的长度，后面存放报文内容。
    // sendBuffer.append(message);              // 把回复报文的内容添加到发送缓冲区。

    // 上面三行代码在Buffer::appendMessage()中实现了，Connection::send()中调用了appendMessage()。
    conn->send(message.data(), message.size());// sendBuffer = message + len，即报文=报文内容+报文头部
}