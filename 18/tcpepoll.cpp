#include "TCPServer.h"  // 这个头文件包含了TCPServer类的定义。

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: %s ip port\n", argv[0]);
        printf("example: ./tcpepoll 172.17.14.95 5005\n\n");
        return -1;
    }
    TCPServer tcpServer(argv[1], atoi(argv[2])); // 创建一个TCPServer对象，参数是IP地址和端口号。
    tcpServer.start(); // 启动服务器，运行事件循环。

    return 0;
}