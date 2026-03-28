#pragma once
#include "Epoll.h"
#include <functional>
#include <sys/syscall.h>
#include <unistd.h>

class Epoll;   // 前向声明，告诉编译器Epoll是一个类，编译器就知道Epoll*是一个指针类型了，就不会报错了
class Channel; // 前向声明，告诉编译器Channel是一个类，编译器就知道Channel*是一个指针类型了，就不会报错了

class EventLoop
{
private:
    Epoll *epoll_; // epoll对象指针
    //  每个事件循环只有一个epoll对象，EventLoop对象中有一个Epoll对象指针成员
    std::function<void(EventLoop *)> epollTimeoutCallback_; // epoll_wait()超时回调函数。

public:
    EventLoop();
    ~EventLoop();
    void run();        // 事件循环
    Epoll *getEpoll(); // 获取epoll对象指针

    void updateChannel(Channel *ch); // 调用Epoll类的updateChannel()函数
    void setEpollTimeoutCallback(std::function<void(EventLoop *)> cb);
};