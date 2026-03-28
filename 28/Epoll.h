#pragma once
#include <sys/epoll.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <errno.h>
#include "Channel.h"

class Channel;  // 前向声明，告诉编译器Channel是一个类，编译器就知道Channel*是一个指针类型了，就不会报错了

class Epoll
{
private:
    static const int MAX_EVENTS = 100; // 最大事件数。
    int epollfd_; // epoll句柄（红黑树）。
    struct epoll_event events_[MAX_EVENTS]; // 存放epoll_wait()返回事件的数组。

public:
    Epoll();
    ~Epoll();
    bool add(int fd, uint32_t events);  // 添加一个fd和它需要监视的事件到epollfd中。
    void updateChannel(Channel *channel);  // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。

    void modify(int fd, uint32_t events);   // 修改一个fd和它需要监视的事件在epollfd中的设置。
    void remove(int fd);    // 从epollfd中删除一个fd。
    int wait(int timeout = -1); // 等待监视的fd有事件发生，返回事件的数量。timeout单位是毫秒，-1表示一直等待。
    struct epoll_event* getEvents() { return events_; } // 获取epoll_wait()返回的事件数组。

    //std::vector<epoll_event> loop(int timeout=-1);   // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
    std::vector<Channel *> loop(int timeout=-1);

};