#include "TCPServer.h"

TCPServer::TCPServer(const char *ip, uint16_t port)
{
    acceptor_ = new Acceptor(&loop_, ip, port); // 在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
}

TCPServer::~TCPServer()
{
    delete acceptor_;
}

void TCPServer::start()
{
    loop_.run(); // 运行事件循环，等待事件的发生，并处理事件。
}