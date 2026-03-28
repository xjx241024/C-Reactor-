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
        close(listenSock.getSockfd());
        return -1;
    }

    if (listenSock.listen() != 0) // 在高并发的网络服务器中，第二个参数要大一些。
    {
        perror("listen() failed");
        close(listenSock.getSockfd());
        return -1;
    }

    Epoll ep; // 创建epoll句柄（红黑树）。

    // 为服务端的listenfd准备读事件。
    if (!ep.add(listenSock.getSockfd(), EPOLLIN)) // 让epoll监视listenfd的读事件，采用水平触发，成功返回true，失败返回false。
    {
        printf("epoll_ctl() failed(%d).\n", errno);
        close(listenSock.getSockfd());
        return -1;
        // exit(-1);
        // exit(-1)会立即终止整个程序的运行，返回给操作系统一个错误码-1；
        // 而return -1只是结束当前函数的执行，并将-1作为返回值返回给调用者，
        // 如果这个函数是main函数，那么return -1也会导致程序终止，但如果这个函数是其他函数，那么程序可能还会继续运行。
    }

    std::vector<epoll_event> evs; // 存放epoll_wait()返回的事件。
    // struct epoll_event evs[10]; // 存放epoll_wait()返回事件的数组。

    while (true) // 事件循环。
    {
        evs = ep.loop(); // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。

        // 如果infds>0，表示有事件发生的fd的数量。
        for (auto &ev : evs) // 遍历epoll返回的事件,注意用引用类型，否则ev是一个临时变量，ev.data.fd的值会被覆盖。
        {
            if (ev.events & EPOLLRDHUP) // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
            {
                printf("client(eventfd=%d) disconnected.\n", ev.data.fd);
                close(ev.data.fd); // 关闭客户端的fd。
            } //  普通数据  带外数据
            else if (ev.events & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
            {
                // listenfd只触发读事件。
                if (ev.data.fd == listenSock.getSockfd()) // 如果是listenfd有事件，表示有新的客户端连上来。
                {
                    InetAddress clientaddr;                       // 客户端地址和协议信息
                    int clientfd = listenSock.accept(clientaddr); // accept4()函数可以直接创建非阻塞的fd
                    Socket *clientSock = new Socket(clientfd);    // 用accept4()函数返回的新连接的fd来构造一个Socket对象。
                    // clientSock只能new出来，不能在栈上，否则析构函数会关闭fd
                    // 如果不用new创建Socket对象，那么它的生命周期就和当前的作用域一样，出了作用域就会被销毁，这样就无法在事件循环中使用它了。

                    // accept()中对clientaddr进行设置
                    printf("accept client(fd=%d,ip=%s,port=%d) ok.\n", clientSock->getSockfd(), clientaddr.ip(), clientaddr.port());

                    // 两种写法都行，但上面的写法更符合面向对象的编程思想，因为我们是通过Socket对象来管理套接字的，而不是直接使用文件描述符。
                    // ep.add(clientfd, EPOLLIN||EPOLLET);
                    ep.add(clientSock->getSockfd(), EPOLLIN || EPOLLET); // 让epoll监视clientfd的读事件，采用边缘触发。
                }
                else
                {
                    char buffer[1024];
                    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
                    {
                        bzero(&buffer, sizeof(buffer));
                        ssize_t nread = read(ev.data.fd, buffer, sizeof(buffer)); // 这行代码用了read()，也可以用recv()，一样的，不要纠结。
                        if (nread > 0)                                            // 成功的读取到了数据。
                        {
                            // 把接收到的报文内容原封不动的发回去。
                            printf("recv(eventfd=%d):%s\n", ev.data.fd, buffer);
                            send(ev.data.fd, buffer, strlen(buffer), 0);
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
                            printf("client(eventfd=%d) disconnected.\n", ev.data.fd);
                            close(ev.data.fd); // 关闭客户端的fd。
                            break;
                        }
                    }
                }
            }
            else if (ev.events & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
            {
            }
            else // 其它事件，都视为错误。
            {
                printf("client(eventfd=%d) error.\n", ev.data.fd);
                close(ev.data.fd); // 关闭客户端的fd。
            }
        }
    }

    return 0;
}