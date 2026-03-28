#include "EchoServer.h"

EchoServer::EchoServer(const char *ip, uint16_t port, int subThreadNum, int workThreadNum)
           :tcpServer_(ip, port, subThreadNum), workThreadPool_(workThreadNum, "Work")
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
    std::cout << "EchoServer::handleNewConnection()运行，\tNew Connection Come In." << std::endl;
    // printf("EchoServer::handleNewConnection() thread is %ld.\n",syscall(SYS_gettid));     // 显示线程ID。
    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleCloseConnection(Connection *conn)
{
    std::cout << "EchoServer::handleCloseConnection()运行，\tEchoServer conn closed." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleErrorConnection(Connection *conn)
{
    std::cout << "EchoServer::handleErrorConnection()运行，\tEchoServer conn error." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

void EchoServer::handleSendComplete(Connection *conn)
{
    std::cout << "EchoServer::handleSendComplete()运行，\tMessage send complete." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

// void EchoServer::handleEpollTimeout(EventLoop *loop)
// {
//     std::cout << "EchoServer timeout." << std::endl;

//     // 根据业务的需求，在这里可以增加其它的代码。
// }

void EchoServer::handleMessage(Connection *conn, std::string &message)
{
    // printf("EchoServer::handleMessage() thread is %ld.\n",syscall(SYS_gettid));     // 显示线程ID。
    
    // 这里不需要占位符，因为能直接传入需要的参数
    // 事实上，onMessage的代码本来就是从handleMessage中移过去的，所以handleMessage当然有对应的参数。
    workThreadPool_.addtask(std::bind(&EchoServer::onMessage, this, conn, message)); // 添加到工作线程池
}

void EchoServer::onMessage(Connection *conn, std::string &message)
{
    // 回显业务
    message = "Reply: " + message; // 构造一个回复报文。
    // sleep(2);
    // printf("处理业务完成，将调用conn");
    conn->send(message.data(), message.size());// sendBuffer = message + len，即报文=报文内容+报文头部
}