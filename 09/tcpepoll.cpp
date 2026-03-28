#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h> // TCP_NODELAY需要包含这个头文件。
#include "InetAddress.h" // 这个头文件包含了InetAddress类的定义。
#include "Socket.h"      // 这个头文件包含了Socket类的定义和createnonblockingsockfd()函数的声明。
#include "Epoll.h"       // 这个头文件包含了Epoll类的定义。
#include "Channel.h"     // 这个头文件包含了Channel类的定义。
#include "EventLoop.h"   // 这个头文件包含了EventLoop类的定义。

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: %s ip port\n", argv[0]);
        printf("example: ./tcpepoll 172.17.14.95 5005\n\n");
        return -1;
    }

    Socket listenSock(createnonblockingsockfd()); // 创建一个非阻塞的套接字文件描述符，并用返回值来构造一个Socket对象。
    listenSock.setreuseaddr(true);                // 设置SO_REUSEADDR选项，允许重用地址。
    listenSock.setreuseport(true);                // 设置SO_REUSEPORT选项
    listenSock.settcpnodelay(true);               // 设置TCP_NODELAY选项，禁止Nagle算法，减少数据包的发送延迟。
    listenSock.setkeepalive(true);                // 设置SO_KEEPALIVE选项，启用TCP keepalive机制，检测死连接。

    InetAddress servaddr(argv[1], atoi(argv[2])); // 创建服务端的InetAddress对象。

    if (listenSock.bind(servaddr) < 0)
    {
        perror("bind() failed");
        close(listenSock.Sockfd());
        return -1;
    }

    if (listenSock.listen() != 0) // 在高并发的网络服务器中，第二个参数要大一些。
    {
        perror("listen() failed");
        close(listenSock.Sockfd());
        return -1;
    }

    // Epoll ep; // 创建epoll句柄（红黑树）。
    EventLoop eLoop; // 创建事件循环对象，EventLoop对象中有一个Epoll对象指针成员，在EventLoop的构造函数中创建一个Epoll对象，并将其地址赋值给这个指针成员，在EventLoop的析构函数中删除这个Epoll对象。
    Channel *listenChannel = new Channel(eLoop.getEpoll(), listenSock.Sockfd()); // 创建一个Channel对象，参数是epoll对象和listenfd。
    listenChannel->setReadCallback(std::bind(&Channel::newConnection, listenChannel, &listenSock)); // 设置listenChannel的读事件回调函数为newConnection函数。
    listenChannel->enablereading(); // 让epoll_wait()监视listenfd的读事件。
    // Channel listenChannel(&ep, listenSock.Sockfd());
    // listenChannel.enablereading();
    // 就目前的程序而言，直接在栈上创建一个Channel对象也是可行的。
    // 为了一致性，一般Channel对象都是用new创建的，所以这里也用new创建一个Channel对象。

    eLoop.run(); // 运行事件循环，等待事件的发生，并处理事件。

    return 0;
}