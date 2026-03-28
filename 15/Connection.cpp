#include "Connection.h"

Connection::Connection(EventLoop *loop, Socket *socket) : loop_(loop), connectionSocket_(socket)
{
    connectionChannel_ = new Channel(loop_, connectionSocket_->Sockfd()); // 用新连接的fd来构造一个Channel对象。
    // clientChannel只能new出来，不能在栈上，否则析构函数会关闭fd
    connectionChannel_->setReadCallback(std::bind(&Channel::onMessage, connectionChannel_)); // 设置clientChannel的读事件回调函数为onMessage函数。
    connectionChannel_->useet(); // 设置边缘触发
    connectionChannel_->enablereading(); // 让epoll_wait()监视clientfd的读事件。
}

Connection::~Connection()
{
    delete connectionChannel_;
    delete connectionSocket_;
    // Channel中new了一个clientSock，但没有释放，由于clientSock的生命周期和Connection一样，所以在Connection的析构函数中释放它。
}