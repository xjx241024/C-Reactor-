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

    for (auto &item : connections_)
    {
        delete item.second; // 释放所有Connection对象的内存。
    }
}

void TCPServer::start()
{
    loop_.run(); // 运行事件循环，等待事件的发生，并处理事件。
}

void TCPServer::onMessage(Connection *conn, std::string message)
{
    message = "Reply: " + message; // 构造一个回复报文。
    int len = message.size();
    std::string sendBuffer((char *)&len, 4); // 构造一个临时发送缓冲区，前4个字节存放报文内容的长度，后面存放报文内容。
    sendBuffer.append(message);              // 把回复报文的内容添加到发送缓冲区。

    // send(conn->fd(), sendBuffer.data(), sendBuffer.size(), 0); // 把发送缓冲区的数据发送回去。
    conn->send(sendBuffer.data(), sendBuffer.size());//sendBuffer里才是要发送的数据，message只是暂时的，要填入sendBuffer的
}

void TCPServer::newConnection(Socket *clientSock)
{
    Connection *conn = new Connection(&loop_, clientSock); // 用新连接的fd来构造一个Connection对象。
    // 注意，这里的conn对象没释放，之后处理。 --- IGNORE ---
    conn->setCloseCallback(std::bind(&TCPServer::closeConnection, this, std::placeholders::_1)); // 设置连接关闭(断开)时的回调函数，参数是TCPServer对象的成员函数closeConnection，绑定this指针和占位符_1。
    conn->setErrorCallback(std::bind(&TCPServer::errorConnection, this, std::placeholders::_1)); // 设置连接发生错误时的回调函数，参数是TCPServer对象的成员函数errorConnection，绑定this指针和占位符_1。
    conn->setOnMessageCallback(std::bind(&TCPServer::onMessage, this, std::placeholders::_1, std::placeholders::_2)); // 设置处理消息的回调函数，参数是TCPServer对象的成员函数onMessage，绑定this指针和占位符_1、_2。

    printf("new connection(fd=%d,ip=%s,port=%d) ok.\n", clientSock->Sockfd(), conn->ip(), conn->port());

    connections_[conn->fd()] = conn; // 将新连接的文件描述符和Connection对象指针添加到connections_ map中。
}

void TCPServer::closeConnection(Connection *conn)
{
    printf("client(eventfd=%d) closed.\n", conn->fd());
    // close(conn->fd()); // Connection对象的析构函数会delete其Socket成员变量，而Socket对象的析构函数会关闭fd，所以这里不需要再调用close()函数来关闭fd了。
    connections_.erase(conn->fd()); // 从connections_ map中删除对应的Connection对象指针。
    delete conn;
}

void TCPServer::errorConnection(Connection *conn)
{
    printf("client(eventfd=%d) error.\n", conn->fd());
    connections_.erase(conn->fd());
    delete conn;
}