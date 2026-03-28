#include "EchoServer.h" 

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: %s ip port\n", argv[0]);
        printf("example: ./server 172.17.14.95 5005\n\n");
        return -1;
    }
    // TCPServer tcpServer(argv[1], atoi(argv[2])); // 创建一个TCPServer对象，参数是IP地址和端口号。
    // tcpServer.start(); // 启动服务器，运行事件循环。

    EchoServer echoServer(argv[1], atoi(argv[2]), 3, 0);
    echoServer.start();

    return 0;
}