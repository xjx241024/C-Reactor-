// 网络通讯的客户端程序。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage:%s ip port\n", argv[0]);
        printf("example:./client 172.17.14.95 5005\n\n");
        return -1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket() failed.\n");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connect(%s:%s) failed.\n", argv[1], argv[2]);
        close(sockfd);
        return -1;
    }

    printf("connect ok.\n");
    // printf("开始时间：%d",time(0));

    for (int ii = 0; ii < 100; ii++)
    {
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "这是第%d个超级女生。", ii);

        char tmpbuf[1024]; // 临时的buffer，报文头部+报文内容。
        memset(tmpbuf, 0, sizeof(tmpbuf));
        int len = strlen(buf);        // 计算报文的大小。
        memcpy(tmpbuf, &len, 4);      // 拼接报文头部。
        memcpy(tmpbuf + 4, buf, len); // 拼接报文内容。

        send(sockfd, tmpbuf, len + 4, 0); // 把请求报文发送给服务端。

        // char tmpbuf[1024]; // 临时缓冲区，报文头部+报文内容。
        // memset(tmpbuf, 0, sizeof(tmpbuf));
        // int headerLen = sprintf(tmpbuf, "Content-Length:%zu\r\n\r\n", strlen(buf)); // 构造报文头部，包含Content-Length字段，指明报文内容的长度。
        // memcpy(tmpbuf + headerLen, buf, strlen(buf)); // 把报文内容复制到临时缓冲区中，紧跟在报文头部之后。
        // send(sockfd, tmpbuf, headerLen + strlen(buf), 0); // 把临时缓冲区中的数据发送给服务端，发送的数据包含报文头部和报文内容。
    }

    for (int ii = 0; ii < 100; ii++)
    {
        int len;
        recv(sockfd, &len, 4, 0); // 先接收4个字节的报文头部，获取报文内容的长度。
        memset(buf, 0, sizeof(buf));
        recv(sockfd, buf, len, 0); // 再接收报文内容。
        printf("recv:%s\n", buf);
    }

    /*     for (int ii = 0; ii < 100; ii++)
        {
            // 从命令行输入内容。
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "hello %d", ii);
            // printf("please input:");
            // scanf("%s", buf);

            if (send(sockfd, buf, strlen(buf), 0) <= 0) // 把命令行输入的内容发送给服务端。
            {
                printf("write() failed.\n");
                close(sockfd);
                return -1;
            }

            memset(buf, 0, sizeof(buf));
            if (recv(sockfd, buf, sizeof(buf), 0) <= 0) // 接收服务端的回应。
            {
                printf("read() failed.\n");
                close(sockfd);
                return -1;
            }

            printf("recv:%s\n", buf);
        } */

    // printf("结束时间：%d",time(0));
}