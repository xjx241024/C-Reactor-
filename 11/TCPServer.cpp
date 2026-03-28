#include "TCPServer.h"

TCPServer::TCPServer(const char *ip, uint16_t port)
{
    Socket *listenSock = new Socket(createnonblockingsockfd()); // 创建一个非阻塞的套接字文件描述符，并用返回值来构造一个Socket对象。
    listenSock->setreuseaddr(true);                // 设置SO_REUSEADDR选项，允许重用地址。
    listenSock->setreuseport(true);                // 设置SO_REUSEPORT选项
    listenSock->settcpnodelay(true);               // 设置TCP_NODELAY选项，禁止Nagle算法，减少数据包的发送延迟。
    listenSock->setkeepalive(true);                // 设置SO_KEEPALIVE选项，启用TCP keepalive机制，检测死连接。

    InetAddress servaddr(ip, port); // 创建服务端的InetAddress对象。

    if (listenSock->bind(servaddr) < 0)
    {
        perror("bind() failed");
        close(listenSock->Sockfd());
        exit(-1);
    }

    if (listenSock->listen() != 0) // 在高并发的网络服务器中，第二个参数要大一些。
    {
        perror("listen() failed");
        close(listenSock->Sockfd());
        exit(-1);
    }

    Channel *listenChannel = new Channel(&loop_, listenSock->Sockfd()); // 创建一个Channel对象，参数是epoll对象和listenfd。
    listenChannel->setReadCallback(std::bind(&Channel::newConnection, listenChannel, listenSock)); // 设置listenChannel的读事件回调函数为newConnection函数。
    listenChannel->enablereading(); // 让epoll_wait()监视listenfd的读事件。
}

TCPServer::~TCPServer()
{

}

void TCPServer::start()
{
    loop_.run(); // 运行事件循环，等待事件的发生，并处理事件。
}