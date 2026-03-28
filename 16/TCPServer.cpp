#include "TCPServer.h"

TCPServer::TCPServer(const char *ip, uint16_t port)
{
    acceptor_ = new Acceptor(&loop_, ip, port); // 在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
    acceptor_->setNewConnectionCallback(std::bind(&TCPServer::newConnection, this, std::placeholders::_1)); 
    // 设置处理新连接的回调函数，参数是TCPServer对象的成员函数newConnection，绑定this指针和占位符_1。
}

TCPServer::~TCPServer()
{
    delete acceptor_;

    for(auto &item : connections_)
    {
        delete item.second; // 释放所有Connection对象的内存。
    }
}

void TCPServer::start()
{
    loop_.run(); // 运行事件循环，等待事件的发生，并处理事件。
}

void TCPServer::newConnection(Socket *clientSock)
{
     Connection *conn = new Connection(&loop_, clientSock); // 用新连接的fd来构造一个Connection对象。
     // 注意，这里的conn对象没释放，之后处理。 --- IGNORE ---
     printf("new connection(fd=%d,ip=%s,port=%d) ok.\n", clientSock->Sockfd(), conn->ip(), conn->port());

     connections_[conn->fd()] = conn; // 将新连接的文件描述符和Connection对象指针添加到connections_ map中。
}