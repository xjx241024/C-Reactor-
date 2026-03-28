#include "Acceptor.h"

Acceptor::Acceptor(EventLoop *loop, const char *ip, uint16_t port):loop_(loop)
{
    acceptSocket_ = new Socket(createnonblockingsockfd()); // 创建一个非阻塞的套接字文件描述符，并用返回值来构造一个Socket对象。
    acceptSocket_->setreuseaddr(true);                // 设置SO_REUSEADDR选项，允许重用地址。
    acceptSocket_->setreuseport(true);                // 设置SO_REUSEPORT选项
    acceptSocket_->settcpnodelay(true);               // 设置TCP_NODELAY选项，禁止Nagle算法，减少数据包的发送延迟。
    acceptSocket_->setkeepalive(true);                // 设置SO_KEEPALIVE选项，启用TCP keepalive机制，检测死连接。

    InetAddress servaddr(ip, port); // 创建服务端的InetAddress对象。

    if (acceptSocket_->bind(servaddr) < 0)
    {
        perror("bind() failed");
        close(acceptSocket_->Sockfd());
        exit(-1);
    }

    if (acceptSocket_->listen() != 0) // 在高并发的网络服务器中，第二个参数要大一些。
    {
        perror("listen() failed");
        close(acceptSocket_->Sockfd());
        exit(-1);
    }

    acceptChannel_ = new Channel(loop_, acceptSocket_->Sockfd()); // 创建一个Channel对象，参数是epoll对象和listenfd。
    acceptChannel_->setReadCallback(std::bind(&Acceptor::newConnection, this)); // 设置listenChannel的读事件回调函数为newConnection函数。
    acceptChannel_->enablereading(); // 让epoll_wait()监视listenfd的读事件。
}

Acceptor::~Acceptor()
{
    // loop_是从TCPServer传入的事件循环对象指针，Acceptor对象不拥有它，不需要在析构函数中删除它。
    delete acceptSocket_;   // 释放套接字对象的内存。
    delete acceptChannel_;  //  释放Channel对象的内存。
}
/* 
void Acceptor::newConnection()
{   
    // 此处是直接调用Channel类的newConnection函数|
    acceptChannel_->newConnection(acceptSocket_); // 调用Channel类的newConnection()函数，接受客户端的连接请求，并将新连接的文件描述符添加到事件循环中。
}
 */

void Acceptor::newConnection()
{
    InetAddress clientaddr;                        // 客户端地址和协议信息
    int clientfd = acceptSocket_->accept(clientaddr); // accept4()函数可以直接创建非阻塞的fd
    Socket *clientSock = new Socket(clientfd);     // 用accept4()函数返回的新连接的fd来构造一个Socket对象。
    // clientSock只能new出来，不能在栈上，否则析构函数会关闭fd
    // 如果不用new创建Socket对象，那么它的生命周期就和当前的作用域一样，出了作用域就会被销毁，这样就无法在事件循环中使用它了。

    clientSock->setipport(clientaddr.ip(), clientaddr.port()); 
    // 这个clientSock是TCPServer::newConnection()中，conn对象获取的IP和端口的来源。

    // 改为用回调函数的方式来处理新连接
    if (newConnectionCallback_) // 如果设置了处理新连接的回调函数，就调用它来处理新连接。
    {
        newConnectionCallback_(clientSock); // 调用处理新连接的回调函数，参数是新连接的Socket对象指针。
    }
    else
    {
        delete clientSock; // 如果没有设置处理新连接的回调函数，就直接删除这个Socket对象，关闭这个连接。
    }
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> cb)
{
    newConnectionCallback_ = cb; // 设置处理新连接的回调函数。
}