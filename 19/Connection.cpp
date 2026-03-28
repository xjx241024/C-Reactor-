#include "Connection.h"

Connection::Connection(EventLoop *loop, Socket *socket) : loop_(loop), connectionSocket_(socket)
{
    connectionChannel_ = new Channel(loop_, connectionSocket_->Sockfd()); // 用新连接的fd来构造一个Channel对象。
    // clientChannel只能new出来，不能在栈上，否则析构函数会关闭fd
    connectionChannel_->setReadCallback(std::bind(&Connection::onMessage, this));      // 设置clientChannel的读事件回调函数为onMessage函数。
    connectionChannel_->setCloseCallback(std::bind(&Connection::closeCallback, this)); // 设置clientChannel的关闭事件回调函数为Connection类的closeCallback函数。
    connectionChannel_->setErrorCallback(std::bind(&Connection::errorCallback, this)); // 设置clientChannel的错误事件回调函数为Connection类的errorCallback函数。
    connectionChannel_->useet();                                                       // 设置边缘触发
    connectionChannel_->enablereading();                                               // 让epoll_wait()监视clientfd的读事件。
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

const char *Connection::ip() const
{
    return connectionSocket_->ip();
}

uint16_t Connection::port() const
{
    return connectionSocket_->port();
}

void Connection::closeCallback()
{
    closeCallback_(this); // 连接关闭(断开)时的回调函数，调用它来处理连接关闭。
}

void Connection::errorCallback()
{
    errorCallback_(this); // 连接发生错误时的回调函数，调用它来处理连接错误。
}

void Connection::setCloseCallback(std::function<void(Connection *)> cb)
{
    closeCallback_ = cb;
}

void Connection::setErrorCallback(std::function<void(Connection *)> cb)
{
    errorCallback_ = cb;
}

void Connection::onMessage()
{
    char buffer[1024];
    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(fd(), buffer, sizeof(buffer)); // 这行代码用了read()，也可以用recv()，一样的，不要纠结。
        if (nread > 0)                                      // 成功的读取到了数据。
        {
            // // 把接收到的报文内容原封不动的发回去。
            // printf("recv(eventfd=%d):%s\n", fd(), buffer);
            // send(fd(), buffer, strlen(buffer), 0);
            inputBuffer_.append(buffer, nread); // 把接收到的数据添加到输入缓冲区。
        }
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {
            continue;
        }
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            printf("recv(eventfd=%d):%s\n", fd(), inputBuffer_.data());
            // 假设数据经过处理，接下来要发送
            outputBuffer_ = inputBuffer_;                              // 这里简单的把输入缓冲区的数据原封不动的复制到输出缓冲区，实际应用中可能会对数据进行处理。
            inputBuffer_.clear();                                      // 处理完数据后，清空输入缓冲区。
            send(fd(), outputBuffer_.data(), outputBuffer_.size(), 0); // 把输出缓冲区的数据发送回去。
            break;
        }
        else if (nread == 0) // 客户端连接已断开。
        {
            // printf("client(eventfd=%d) disconnected.\n", fd());
            // close(fd()); // 关闭客户端的fd。
            closeCallback(); // 连接关闭(断开)时的回调函数，调用它来处理连接关闭。
            break;
        }
    }
}