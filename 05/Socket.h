#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/tcp.h> // TCP_NODELAY需要包含这个头文件。
#include "InetAddress.h"

int createnonblockingsockfd();  // 创建一个非阻塞的套接字文件描述符。

class Socket
{
private:
    const int sockfd_; // 套接字的文件描述符,使用const修饰，表示在构造函数中初始化后不能修改。
public:
    Socket(int sockfd); // 构造函数

    void setreuseaddr(bool on);       // 设置SO_REUSEADDR选项，true-打开，false-关闭。
    void setreuseport(bool on);       // 设置SO_REUSEPORT选项。
    void settcpnodelay(bool on);     // 设置TCP_NODELAY选项。
    void setkeepalive(bool on);       // 设置SO_KEEPALIVE选项。

    int getSockfd() const; // 获取套接字文件描述符的成员函数。

    int bind(const InetAddress& serveraddr); // 绑定套接字到指定的InetAddress地址。
    int listen(int backlog = 128); // 监听套接字
    int accept(InetAddress& clientaddr, int flags = SOCK_NONBLOCK); // flags参数默认值为SOCK_NONBLOCK，表示创建的套接字是非阻塞的。

    ~Socket(); // 析构函数
};