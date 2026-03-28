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

    Epoll ep; // 创建epoll句柄（红黑树）。

    Channel *listenChannel = new Channel(&ep, listenSock.Sockfd(), true); // 创建一个Channel对象，参数是epoll对象和listenfd。
    listenChannel->enablereading(); // 让epoll_wait()监视listenfd的读事件。
    // Channel listenChannel(&ep, listenSock.Sockfd());
    // listenChannel.enablereading();
    // 就目前的程序而言，直接在栈上创建一个Channel对象也是可行的。
    // 为了一致性，一般Channel对象都是用new创建的，所以这里也用new创建一个Channel对象。

    std::vector<Channel *> channels; // 存放epoll_wait()返回的事件,每个事件对应一个Channel对象，Channel对象中有fd和需要监视的事件。

    while (true) // 事件循环。
    {
        channels = ep.loop(); // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。

        // 如果infds>0，表示有事件发生的fd的数量。
        for (auto &ch : channels) // 遍历epoll返回的事件
        {
            ch->handleevents(&listenSock); // 处理事件的函数
/*             if (ch->getevents() & EPOLLRDHUP) // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
            {
                printf("client(eventfd=%d) disconnected.\n", ch->fd());
                close(ch->fd()); // 关闭客户端的fd。
            } //  普通数据  带外数据
            else if (ch->getevents() & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
            {
                // 此处从判断fd改为判断Channel对象，因为Channel对象中有fd和需要监视的事件。
                if (ch == listenChannel) // 如果是listenfd有事件，表示有新的客户端连上来。listenfd只触发读事件
                {
                    InetAddress clientaddr;                       // 客户端地址和协议信息
                    int clientfd = listenSock.accept(clientaddr); // accept4()函数可以直接创建非阻塞的fd
                    Socket *clientSock = new Socket(clientfd);    // 用accept4()函数返回的新连接的fd来构造一个Socket对象。
                    // clientSock只能new出来，不能在栈上，否则析构函数会关闭fd
                    // 如果不用new创建Socket对象，那么它的生命周期就和当前的作用域一样，出了作用域就会被销毁，这样就无法在事件循环中使用它了。

                    // accept()中对clientaddr进行设置
                    printf("accept client(fd=%d,ip=%s,port=%d) ok.\n", clientSock->Sockfd(), clientaddr.ip(), clientaddr.port());

                    // Channel和Socket是多对一的关系，一个Channel对象对应一个fd，而一个fd可以有多个Channel对象监视不同的事件。
                    // 这部分代码新建了客户端socket，所以需要新建一个Channel对象来监视这个客户端socket的事件。
                    Channel *clientChannel = new Channel(&ep, clientSock->Sockfd(), false); // 用新连接的fd来构造一个Channel对象。
                    // clientChannel只能new出来，不能在栈上，否则析构函数会关闭fd
                    clientChannel->enablereading(); // 让epoll_wait()监视clientfd的读事件。
                }
                else
                {
                    char buffer[1024];
                    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
                    {
                        bzero(&buffer, sizeof(buffer));
                        ssize_t nread = read(ch->fd(), buffer, sizeof(buffer)); // 这行代码用了read()，也可以用recv()，一样的，不要纠结。
                        if (nread > 0)                                            // 成功的读取到了数据。
                        {
                            // 把接收到的报文内容原封不动的发回去。
                            printf("recv(eventfd=%d):%s\n", ch->fd(), buffer);
                            send(ch->fd(), buffer, strlen(buffer), 0);
                        }
                        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
                        {
                            continue;
                        }
                        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
                        {
                            break;
                        }
                        else if (nread == 0) // 客户端连接已断开。
                        {
                            printf("client(eventfd=%d) disconnected.\n", ch->fd());
                            close(ch->fd()); // 关闭客户端的fd。
                            break;
                        }
                    }
                }
            }
            else if (ch->getrevents() & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
            {
            }
            else // 其它事件，都视为错误。
            {
                printf("client(eventfd=%d) error.\n", ch->fd());
                close(ch->fd()); // 关闭客户端的fd。
            } */
        }
    }

    return 0;
}