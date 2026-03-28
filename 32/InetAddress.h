#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

//socket的地址协议类，包含了IP地址和端口号。
class InetAddress
{
private:
    struct sockaddr_in addr_;
public:
    InetAddress();
    // 此处ip参数的类型用std::string也行，不过const char*更加简洁
    // port参数的类型用int也行，不过uint16_t更合适，因为端口号的范围是0-65535。
    InetAddress(const char *ip, uint16_t port); // 监听用这个
    InetAddress(const sockaddr_in &addr); // 客户端连接用这个

    const char* ip() const; // 获取字符串表示的IP地址字符串。
    uint16_t port() const;  // 获取整数表示的端口号，返回值是主机字节序的端口号。
    const struct sockaddr * getSockAddr() const;    // 获取sockaddr_in结构体的地址，返回值类型是sockaddr*，因为有些函数需要这个类型的参数。
    void setaddr(const sockaddr_in &addr); // 设置sockaddr_in结构体的地址，直接复制整个结构体的内容。
    ~InetAddress();
};

