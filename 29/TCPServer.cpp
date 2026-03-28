#include "TCPServer.h"

TCPServer::TCPServer(const char *ip, uint16_t port, int threadNum):threadNum_(threadNum)
{
    mainLoop_.setEpollTimeoutCallback(std::bind(&TCPServer::epollTimeout, this, std::placeholders::_1));

    acceptor_ = new Acceptor(&mainLoop_, ip, port); // 在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
    acceptor_->setNewConnectionCallback(std::bind(&TCPServer::newConnection, this, std::placeholders::_1)); // 设置处理新连接的回调函数

    threadPool_ = new ThreadPool(threadNum_, "IO");   // 创建线程池

    // 创建从事件的循环
    for(int i =0; i < threadNum_; i++)
    {
        subLoops_.push_back(new EventLoop); // 创建从事件并放入容器中
        subLoops_[i]->setEpollTimeoutCallback(std::bind(&TCPServer::epollTimeout, this, std::placeholders::_1));// 设置超时回调函数
        threadPool_->addtask(std::bind(&EventLoop::run, subLoops_[i])); // 在线程池中运行事件循环
    }
}

TCPServer::~TCPServer()
{
    delete acceptor_;

    /*  智能指针，会自动释放，所以这部分代码不需要了
    for (auto &item : connections_)
    {
        delete item.second; // 释放所有Connection对象的内存。
    } 
    */

    for (auto &item : subLoops_)
    {
        delete item; // 释放所有从事件循环的内存。
    }

    delete threadPool_;
}

void TCPServer::start()
{
    mainLoop_.run(); // 运行事件循环，等待事件的发生，并处理事件。
}

void TCPServer::onMessage(spConnection conn, std::string &message)
{
    if(onMessageCallback_)
        onMessageCallback_(conn, message); // 回调EchoServer::handleMessage
    // 原本代码已经移动到EchoServer中
    /*     message = "Reply: " + message; // 构造一个回复报文。
        int len = message.size();
        std::string sendBuffer((char *)&len, 4); // 构造一个临时发送缓冲区，前4个字节存放报文内容的长度，后面存放报文内容。
        sendBuffer.append(message);              // 把回复报文的内容添加到发送缓冲区。

        conn->send(sendBuffer.data(), sendBuffer.size());// sendBuffer = message + len，即报文=报文内容+报文头部 */
}

void TCPServer::newConnection(Socket* clientSock)
{
    // Connection *conn = new Connection(&mainLoop_, clientSock); // 用新连接的fd来构造一个Connection对象。
    // 将新建的conn分配给从事件循环
    // Connection *conn = new Connection(subLoops_[clientSock->Sockfd() % threadNum_], clientSock);
    spConnection conn(new Connection(subLoops_[clientSock->Sockfd() % threadNum_], clientSock));// 智能指针对象创建
    conn->setCloseCallback(std::bind(&TCPServer::closeConnection, this, std::placeholders::_1));                      // 设置连接关闭(断开)时的回调函数，参数是TCPServer对象的成员函数closeConnection，绑定this指针和占位符_1。
    conn->setErrorCallback(std::bind(&TCPServer::errorConnection, this, std::placeholders::_1));                      // 设置连接发生错误时的回调函数，参数是TCPServer对象的成员函数errorConnection，绑定this指针和占位符_1。
    conn->setOnMessageCallback(std::bind(&TCPServer::onMessage, this, std::placeholders::_1, std::placeholders::_2)); // 设置处理消息的回调函数，参数是TCPServer对象的成员函数onMessage，绑定this指针和占位符_1、_2。
    conn->setSendCompleteCallback(std::bind(&TCPServer::sendComplete, this, std::placeholders::_1));                  // 发送完成时的回调函数

    // printf("new connection(fd=%d,ip=%s,port=%d) ok.\n", clientSock->Sockfd(), conn->ip(), conn->port());

    connections_[conn->fd()] = conn; // 将新连接的文件描述符和Connection对象指针添加到connections_ map中。

    if(newConnectionCallback_)
        newConnectionCallback_(conn); // 连接建立后再回调，回调EchoServer::handleNewConnection
}

void TCPServer::closeConnection(spConnection conn)
{
    if(closeConnectionCallback_)
        closeConnectionCallback_(conn); // 先回调，再关闭连接。回调EchoServer::handleCloseConnection

    // printf("client(eventfd=%d) closed.\n", conn->fd());
    // close(conn->fd()); // Connection对象的析构函数会delete其Socket成员变量，而Socket对象的析构函数会关闭fd，所以这里不需要再调用close()函数来关闭fd了。
    connections_.erase(conn->fd()); // 从connections_ map中删除对应的Connection对象指针。
    // delete conn;
}

void TCPServer::errorConnection(spConnection conn)
{
    if(errorConnectionCallback_)
        errorConnectionCallback_(conn); // 先回调，再关闭连接。回调EchoServer::handleErrorConnection

    // printf("client(eventfd=%d) error.\n", conn->fd());
    connections_.erase(conn->fd());
    // delete conn;
}

void TCPServer::sendComplete(spConnection conn)
{
    // printf("send completed\n");
    if(sendCompleteCallback_)
        sendCompleteCallback_(conn); // 回调EchoServer::handleSendComplete
    // 业务通过回调函数，在EchoServer完成
}

void TCPServer::epollTimeout(EventLoop *loop)
{
    // printf("epoll_wait() timeout.\n");
    if (epollTimeoutCallback_)      // 检查是否包含有效的可调用目标（是否赋值）
        epollTimeoutCallback_(loop); // 回调EchoServer::handleEpollTimeout
    // 业务通过回调函数，在EchoServer完成
}

void TCPServer::setOnMessageCallback(std::function<void(spConnection, std::string &message)> cb)
{
    onMessageCallback_ = cb;
}

void TCPServer::setSendCompleteCallback(std::function<void(spConnection)> cb)
{
    sendCompleteCallback_ = cb;
}

void TCPServer::setCloseConnectionCallback(std::function<void(spConnection)> cb)
{
    closeConnectionCallback_ = cb;
}

void TCPServer::setErrorConnectionCallback(std::function<void(spConnection)> cb)
{
    errorConnectionCallback_ = cb;
}

void TCPServer::setEpollTimeoutCallback(std::function<void(EventLoop*)> cb)
{
    epollTimeoutCallback_ = cb;
}

void TCPServer::setNewConnectionCallback(std::function<void(spConnection)> cb)
{
    newConnectionCallback_ = cb;
}