#include <signal.h>
#include "EchoServer.h" 

EchoServer *echoServer; // 全局对象

void stop(int signal)   // 接收信号2和15，停止程序
{
    printf("%d\n", signal);
    echoServer->stop();
    delete echoServer;
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: %s ip port\n", argv[0]);
        printf("example: ./echoserver 172.17.14.95 5005\n\n");
        return -1;
    }

    // stop函数处理信号15和2
    signal(SIGTERM, stop);  // 信号15，kill或killall命令的默认发送信号
    signal(SIGINT, stop);   // 信号2，ctrl+c

    echoServer = new EchoServer(argv[1], atoi(argv[2]), 3, 0);  // 简单的回显业务，工作线程池少运行更快
    echoServer->start();

    return 0;
}
