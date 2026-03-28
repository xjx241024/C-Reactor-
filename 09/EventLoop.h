#pragma once
#include "Epoll.h"

class EventLoop
{
private:
    Epoll *epoll_; // epoll对象指针
    //  每个事件循环只有一个epoll对象，EventLoop对象中有一个Epoll对象指针成员

public:
    EventLoop();
    ~EventLoop();
    void run(); // 事件循环
    Epoll *getEpoll(); // 获取epoll对象指针
};