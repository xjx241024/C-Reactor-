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


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: %s ip port\n", argv[0]);
        printf("example: ./tcpepoll 172.17.14.95 5005\n\n");
        return -1;
    }

    // 创建服务端用于监听的listenfd。SOCK_NONBLOCK选项，创建的listenfd就是非阻塞的了。
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0)
    {
        perror("socket() failed");
        return -1;
    }

    // 设置listenfd的属性，如果对这些属性不熟悉，百度之。
    int opt = 1;
    // 设置SO_REUSEADDR选项，允许重用地址。
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt)); // 必须的。
    // TCP_NODELAY选项，禁止Nagle算法，减少数据包的发送延迟。
    setsockopt(listenfd, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt)); // 必须的。
    // SO_REUSEPORT选项，允许多个套接字绑定到同一个地址和端口，负载均衡。
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt)); // 有用，但是，在Reactor中意义不大。
    // SO_KEEPALIVE选项，启用TCP keepalive机制，检测死连接。
    setsockopt(listenfd, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt)); // 可能有用，但是，建议自己做心跳。

    InetAddress servaddr(argv[1], atoi(argv[2])); // 创建服务端的InetAddress对象。

    /*     
    struct sockaddr_in servaddr;                   // 服务端地址的结构体。
    servaddr.sin_family = AF_INET;                 // IPv4网络协议的套接字类型。
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // 服务端用于监听的ip地址。
    servaddr.sin_port = htons(atoi(argv[2]));      // 服务端用于监听的端口。 
    */

    if (bind(listenfd, servaddr.getSockAddr(), sizeof(sockaddr)) < 0)
    {
        perror("bind() failed");
        close(listenfd);
        return -1;
    }

    if (listen(listenfd, 128) != 0) // 在高并发的网络服务器中，第二个参数要大一些。
    {
        perror("listen() failed");
        close(listenfd);
        return -1;
    }

    int epollfd = epoll_create(1); // 创建epoll句柄（红黑树）。

    // 为服务端的listenfd准备读事件。
    struct epoll_event ev; // 声明事件的数据结构体。
    ev.data.fd = listenfd; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events = EPOLLIN;   // 让epoll监视listenfd的读事件，采用水平触发。

    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev); // 把需要监视的listenfd和它的事件加入epollfd中。

    struct epoll_event evs[10]; // 存放epoll_wait()返回事件的数组。

    while (true) // 事件循环。
    {
        int infds = epoll_wait(epollfd, evs, 10, -1); // 等待监视的fd有事件发生。

        // 返回失败。
        if (infds < 0)
        {
            perror("epoll_wait() failed");
            break;
        }

        // 超时。
        if (infds == 0)
        {
            printf("epoll_wait() timeout.\n");
            continue;
        }

        // 如果infds>0，表示有事件发生的fd的数量。
        for (int ii = 0; ii < infds; ii++) // 遍历epoll返回的数组evs。
        {
            if (evs[ii].events & EPOLLRDHUP) // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
            {
                printf("client(eventfd=%d) disconnected.\n", evs[ii].data.fd);
                close(evs[ii].data.fd); // 关闭客户端的fd。
            } //  普通数据  带外数据
            else if (evs[ii].events & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
            {
                // listenfd只触发读事件。
                if (evs[ii].data.fd == listenfd) // 如果是listenfd有事件，表示有新的客户端连上来。
                {
                    struct sockaddr_in peeraddr;
                    socklen_t len = sizeof(peeraddr);
                    int clientfd = accept4(listenfd, (struct sockaddr *)&peeraddr, &len, SOCK_NONBLOCK);
                    // accept4()函数可以直接创建非阻塞的fd。
                    
                    InetAddress clientaddr(peeraddr); // 创建一个InetAddress对象，保存新连接的客户端地址信息。

                    printf("accept client(fd=%d,ip=%s,port=%d) ok.\n", clientfd, clientaddr.ip(), clientaddr.port());

                    // 为新客户端连接准备读事件，并添加到epoll中。
                    ev.data.fd = clientfd;
                    ev.events = EPOLLIN | EPOLLET; // 边缘触发。
                    epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &ev);
                }
                else
                {
                    char buffer[1024];
                    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
                    {
                        bzero(&buffer, sizeof(buffer));
                        ssize_t nread = read(evs[ii].data.fd, buffer, sizeof(buffer)); // 这行代码用了read()，也可以用recv()，一样的，不要纠结。
                        if (nread > 0)                                                 // 成功的读取到了数据。
                        {
                            // 把接收到的报文内容原封不动的发回去。
                            printf("recv(eventfd=%d):%s\n", evs[ii].data.fd, buffer);
                            send(evs[ii].data.fd, buffer, strlen(buffer), 0);
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
                            printf("client(eventfd=%d) disconnected.\n", evs[ii].data.fd);
                            close(evs[ii].data.fd); // 关闭客户端的fd。
                            break;
                        }
                    }
                }
            }
            else if (evs[ii].events & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
            {
            }
            else // 其它事件，都视为错误。
            {
                printf("client(eventfd=%d) error.\n", evs[ii].data.fd);
                close(evs[ii].data.fd); // 关闭客户端的fd。
            }
        }
    }

    return 0;
}