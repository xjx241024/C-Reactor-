#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Buffer.h"
#include "Timestamp.h"  // 时间戳
#include <functional>
#include <memory>
#include <atomic>
#include <sys/syscall.h>

class Connection;                                 // 前置声明Connection类，因为设置别名时Connection类还没定义，所以要先声明。
using spConnection = std::shared_ptr<Connection>; // 取个别名，不然写起来太长了

class Connection : public std::enable_shared_from_this<Connection>
{
private:
    EventLoop *loop_;                            // Connection对应的事件循环，从构造函数传入，来自TCPServer。
    std::unique_ptr<Socket> connectionSocket_;   // Connection对应的套接字，负责与客户端进行通信。
    std::unique_ptr<Channel> connectionChannel_; // Connection对应的Channel，负责监视连接套接字的读写事件。
    std::atomic_bool disconnect_;                // IO线程中改变这个成员变量的值，工作线程会判断这个成员变量的值

    std::function<void(spConnection)> closeCallback_;                    // 连接关闭(断开)时的回调函数。
    std::function<void(spConnection)> errorCallback_;                    // 连接发生错误时的回调函数。
    std::function<void(spConnection, std::string &)> onMessageCallback_; // 处理消息的回调函数，参数是一个无参数、无返回值的函数对象。
    std::function<void(spConnection)> sendComplteCallback_;              // 发送完成的函调函数

    Buffer inputBuffer_;  // 输入缓冲区
    Buffer outputBuffer_; // 输出缓冲区

    Timestamp lastTime_; // 时间戳，初始值是创建Connection对象的时间，每接收到一个报文，更新为当前时间

public:
    Connection(EventLoop *loop, std::unique_ptr<Socket> socket); // 构造函数，参数是事件循环对象指针和套接字对象指针，在构造函数中创建一个非阻塞的套接字文件描述符，并用返回值来构造一个Socket对象。
    ~Connection();

    int fd() const;         // 获取连接套接字的文件描述符。
    const char *ip() const; // 获取连接套接字的IP地址字符串。
    uint16_t port() const;  // 获取连接套接字的端口号

    void closeCallback(); // 连接关闭(断开)时的回调函数
    void errorCallback(); // 连接发生错误时的回调函数
    void writeCallback(); // 写事件的回调函数，发送消息的函数send()会注册写事件，一旦socket可写立刻发送数据，发送完成后取消写事件。

    void setCloseCallback(std::function<void(spConnection)> cb);                    // 设置连接关闭(断开)时的回调函数。
    void setErrorCallback(std::function<void(spConnection)> cb);                    // 设置连接发生错误时的回调函数。
    void setOnMessageCallback(std::function<void(spConnection, std::string &)> cb); // 设置处理消息的回调函数，参数是一个无参数、无返回值的函数对象。
    void setSendCompleteCallback(std::function<void(spConnection)> cb);             // 处理发送完成事件的回调函数

    void onMessage();                       // 处理消息的函数
    void send(const char *msg, size_t len); // 发送消息的函数，参数是要发送的消息字符串。无论在什么线程发送，都调用此函数
    // void sendInLoop(const char *msg, size_t len); 错误示范
    void sendInLoop(std::shared_ptr<std::string> msg); // 在IO线程则直接调用，在工作线程则将此函数传给IO线程调用
};