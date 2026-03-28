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
        printf("usage:./client ip port\n"); 
        printf("example:./client 172.17.14.95 5005\n\n"); 
        return -1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char buf[1024];
 
    if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0) { printf("socket() failed.\n"); return -1; }
    
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr=inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
    {
        printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(sockfd);  return -1;
    }

    printf("connect ok.\n");
    // printf("开始时间：%d",time(0));

    for (int ii=0;ii<1000;ii++)
    {
        // 报文头部+报文内容
        memset(buf,0,sizeof(buf));
        sprintf(buf,"这是第%d个超级女生。",ii);   // 此处由%d改为%03d，这样每个报文的长度都相同

        int len = strlen(buf);  // 存储报文长度的整数作为报文头部
        char tmpbuf[1028];  // 存储要发送的报文，包括报文头部+报文内容(4+1024=1028)
        memset(tmpbuf, 0, sizeof(tmpbuf));  // 清空tmpbuf，防止发送未知的内容
        memcpy(tmpbuf, &len, 4);    // 将报文头部(4字节的整数)放入报文
        memcpy(tmpbuf + 4, buf, len);   // 将报文内容放入报文

        if (send(sockfd,tmpbuf,len+4,0) <=0)       // 把报文发送给服务端。
        { 
            printf("write() failed.\n");  close(sockfd);  return -1;
        }
        
        // usleep(100);
        /*
        memset(buf,0,sizeof(buf));
        if (recv(sockfd,buf,sizeof(buf),0) <=0)      // 接收服务端的回应。
        { 
            printf("read() failed.\n");  close(sockfd);  return -1;
        }

        printf("recv:%s\n",buf);
        */
    }

    // printf("结束时间：%d",time(0));
} 