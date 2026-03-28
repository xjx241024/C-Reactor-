#include "Epoll.h"

Epoll::Epoll()
{
    epollfd_ = epoll_create(1); // 创建epoll句柄（红黑树）。
}

bool Epoll::add(int fd, uint32_t events)
{
    struct epoll_event ev; // 声明事件的数据结构体。
    ev.data.fd = fd; // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events = events;   // 让epoll监视fd的events事件。

    if (epoll_ctl(epollfd_,EPOLL_CTL_ADD,fd,&ev)==-1)     // 把需要监视的fd和它的事件加入epollfd中。
    {
        return false;
    }
    return true;
}

// 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。
std::vector<epoll_event> Epoll::loop(int timeout)   
{
    std::vector<epoll_event> evs;        // 存放epoll_wait()返回的事件。

    bzero(events_,sizeof(events_)); // 清空events_数组，避免epoll_wait()返回的事件被上次的事件覆盖。
    int infds=epoll_wait(epollfd_,events_,MAX_EVENTS,timeout);       // 等待监视的fd有事件发生。

    // 返回失败。
    if (infds < 0)
    {
        perror("epoll_wait() failed"); exit(-1);
    }

    // 超时。
    if (infds == 0)
    {
        printf("epoll_wait() timeout.\n"); return evs;
    }

    // 如果infds>0，表示有事件发生的fd的数量。
    for (int ii=0;ii<infds;ii++)       // 遍历epoll返回的数组events_。
    {
        evs.push_back(events_[ii]);
    }

    return evs;
}

Epoll::~Epoll()
{
    ::close(epollfd_);
}