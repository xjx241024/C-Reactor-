#include "Epoll.h"

Epoll::Epoll()
{
    epollfd_ = epoll_create(1); // 创建epoll句柄（红黑树）。
}

Epoll::~Epoll()
{
    ::close(epollfd_);
}

void Epoll::updateChannel(Channel *ch)
{
    struct epoll_event ev;
    ev.data.ptr = ch;            // 指定channel
    ev.events = ch->getevents(); // 让epoll监视fd的events事件。

    if (ch->getinpoll()) // 如果channel已经在红黑树上了，就修改它的事件。
    {
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev) == -1) // 把需要监视的fd和它的事件加入epollfd中。
        {
            printf("epoll_ctl() failed(%d).\n", errno);
            exit(-1);
        }
    }
    else // 如果channel还没有在红黑树上，就添加它到红黑树上。
    {
        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev) == -1) // 把需要监视的fd和它的事件加入epollfd中。
        {
            printf("epoll_ctl() failed(%d).\n", errno);
            exit(-1);
        }
        ch->setinepoll(); // 把channel的inepoll_成员设置为true。
    }
}

bool Epoll::add(int fd, uint32_t events)
{
    struct epoll_event ev; // 声明事件的数据结构体。
    ev.data.fd = fd;       // 指定事件的自定义数据，会随着epoll_wait()返回的事件一并返回。
    ev.events = events;    // 让epoll监视fd的events事件。

    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev) == -1) // 把需要监视的fd和它的事件加入epollfd中。
    {
        return false;
    }
    return true;
}

std::vector<Channel *> Epoll::loop(int timeout)
{
    std::vector<Channel *> channels; // 存放epoll_wait()返回的事件对应的channel。

    bzero(events_, sizeof(events_));                                // 清空events_数组，避免epoll_wait()返回的事件被上次的事件覆盖。
    int infds = epoll_wait(epollfd_, events_, MAX_EVENTS, timeout); // 等待监视的fd有事件发生。

    // 返回失败。
    if (infds < 0)
    {
        // EBADF ：epfd不是一个有效的描述符。
        // EFAULT ：参数events指向的内存区域不可写。
        // EINVAL ：epfd不是一个epoll文件描述符，或者参数maxevents小于等于0。
        // EINTR ：阻塞过程中被信号中断，epoll_pwait()可以避免，或者错误处理中，解析error后重新调用epoll_wait()。
        // 代码无BUG，就不会出现前三种情况。
        // 在Reactor模型中，不建议使用信号，因为信号处理起来很麻烦，没有必要。------ 陈硕
        perror("epoll_wait() failed");
        exit(-1);
    }

    // 超时。
    if (infds == 0)
    {
        // 如果epoll_wait()超时，表示系统很空闲，返回的channels将为空。
        return channels;
    }

    // 如果infds>0，表示有事件发生的fd的数量。
    for (int ii = 0; ii < infds; ii++) // 遍历epoll返回的数组events_。
    {
        Channel *ch = (Channel *)events_[ii].data.ptr; // 获取事件对应的channel。
        ch->setrevents(events_[ii].events);            // 把事件的类型保存到channel的revents_成员中，供后续处理事件的时候使用。
        channels.push_back(ch);                        // 把channel添加到channels容器中，返回给调用者。
    }

    return channels;
}