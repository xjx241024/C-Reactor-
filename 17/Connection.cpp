#include "Connection.h"

Connection::Connection(EventLoop *loop, Socket *socket) : loop_(loop), connectionSocket_(socket)
{
    connectionChannel_ = new Channel(loop_, connectionSocket_->Sockfd()); // 用新连接的fd来构造一个Channel对象。
    // clientChannel只能new出来，不能在栈上，否则析构函数会关闭fd
    connectionChannel_->setReadCallback(std::bind(&Channel::onMessage, connectionChannel_)); // 设置clientChannel的读事件回调函数为onMessage函数。
    connectionChannel_->setCloseCallback(std::bind(&Connection::closeCallback, this)); // 设置clientChannel的关闭事件回调函数为Connection类的closeCallback函数。
    connectionChannel_->setErrorCallback(std::bind(&Connection::errorCallback, this)); // 设置clientChannel的错误事件回调函数为Connection类的errorCallback函数。
    connectionChannel_->useet(); // 设置边缘触发
    connectionChannel_->enablereading(); // 让epoll_wait()监视clientfd的读事件。
}

Connection::~Connection()
{
    delete connectionChannel_;
    delete connectionSocket_;
    // Channel中new了一个clientSock，但没有释放，由于clientSock的生命周期和Connection一样，所以在Connection的析构函数中释放它。
}

int Connection::fd() const
{
    return connectionSocket_->Sockfd();
}

const char* Connection::ip() const
{
    return connectionSocket_->ip();
}

uint16_t Connection::port() const
{
    return connectionSocket_->port();
}

void Connection::closeCallback()
{
    printf("client(eventfd=%d) closed.\n", fd());
    close(fd()); // 关闭客户端的fd。
}

void Connection::errorCallback()
{
    printf("client(eventfd=%d) error.\n", fd());
    close(fd()); // 关闭客户端的fd。
}