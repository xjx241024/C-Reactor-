#include "Socket.h"

int createnonblockingsockfd()
{
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (sockfd < 0)
    {
        // perror("socket() failed");
        printf("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        exit(-1);
    }
    return sockfd;
}

Socket::Socket(int sockfd) : sockfd_(sockfd)
{
    // 将传入的sockfd参数赋值给成员变量sockfd。
}

void Socket::setreuseaddr(bool on)
{
    int opt = on ? 1 : 0; // 将bool类型的参数转换为int类型，true对应1，false对应0。
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, static_cast<socklen_t>(sizeof opt));
}

void Socket::setreuseport(bool on)
{
    int opt = on ? 1 : 0; // 将bool类型的参数转换为int类型，true对应1，false对应0。
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, static_cast<socklen_t>(sizeof opt));
}

void Socket::settcpnodelay(bool on)
{
    int opt = on ? 1 : 0; // 将bool类型的参数转换为int类型，true对应1，false对应0。
    ::setsockopt(sockfd_, SOL_SOCKET, TCP_NODELAY, &opt, static_cast<socklen_t>(sizeof opt));
}

void Socket::setkeepalive(bool on)
{
    int opt = on ? 1 : 0; // 将bool类型的参数转换为int类型，true对应1，false对应0。
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, static_cast<socklen_t>(sizeof opt));
}

int Socket::Sockfd() const
{
    return sockfd_; // 返回套接字文件描述符的成员变量sockfd_。
}

const char *Socket::ip() const
{
    return ip_; // 返回字符串表示的IP地址字符串的成员变量ip_。
}

uint16_t Socket::port() const
{
    return port_;   // 返回整数表示的端口号的成员变量port_。
}

void Socket::setipport(const char *ip, uint16_t port)
{
    ip_ = ip;       // 将输入的字符串表示的IP地址赋值给成员变量ip_。
    port_ = port;   // 将输入的整数表示的端口号赋值给成员变量port_。
}

int Socket::bind(const InetAddress &serveraddr)
{
    setipport(serveraddr.ip(), serveraddr.port()); // 将InetAddress对象的ip()函数返回的字符串表示的IP地址赋值给成员变量ip_，将InetAddress对象的port()函数返回的整数表示的端口号赋值给成员变量port_。
    return ::bind(sockfd_, serveraddr.getSockAddr(), sizeof(sockaddr));
}

int Socket::listen(int backlog)
{
    return ::listen(sockfd_, backlog);
}

int Socket::accept(InetAddress &clientaddr, int flags)
{
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientfd = ::accept4(sockfd_, (struct sockaddr *)&peeraddr, &len, SOCK_NONBLOCK);

    clientaddr.setaddr(peeraddr);

    return clientfd;
}

Socket::~Socket()
{
    ::close(sockfd_); // 在析构函数中关闭套接字，释放资源。
}