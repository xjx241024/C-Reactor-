#include "InetAddress.h"

InetAddress::InetAddress()
{
    // 默认构造函数，初始化addr_结构体为0。
    memset(&addr_, 0, sizeof(addr_));
}

InetAddress::InetAddress(const char *ip, uint16_t port)
{
    addr_.sin_family = AF_INET;        // IPv4网络协议的套接字类型。
    addr_.sin_addr.s_addr = inet_addr(ip); // 将字符串表示的IP地址转换为网络字节序的整数，并赋值给addr_结构体的sin_addr成员。
    addr_.sin_port = htons(port);      // 将主机字节序的端口号转换为网络字节序，并赋值给addr_结构体的sin_port成员。
}

InetAddress::InetAddress(const sockaddr_in &addr):addr_(addr)
{
    // 用sockaddr_in来构造InetAddress对象，直接复制整个结构体的内容
    // 直接用初始化列表完成复制
}

InetAddress::~InetAddress()
{
    // 目前没有需要释放的资源，所以析构函数是空的。
}

// 获取字符串表示的IP地址字符串
const char* InetAddress::ip() const
{
    return inet_ntoa(addr_.sin_addr); // 将网络字节序的IP地址转换为字符串表示，并返回。
}

// 获取整数表示的端口号，返回值是主机字节序的端口号
uint16_t InetAddress::port() const
{
    return ntohs(addr_.sin_port); // 将网络字节序的端口号转换为主机字节序，并返回。
}

// 获取sockaddr_in结构体的地址，返回值类型
const struct sockaddr * InetAddress::getSockAddr() const
{
    // 问题：需要加const吗？
    // 答：因为成员函数是const的，所以返回值也应该是const的。
    return (const struct sockaddr *) &addr_; // 将成员变量addr_的地址转换为sockaddr*类型，并返回。
}

void InetAddress::setaddr(const sockaddr_in &addr)
{
    addr_ = addr; // 直接复制整个结构体的内容。
}