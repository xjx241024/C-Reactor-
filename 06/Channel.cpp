#include "Channel.h"

Channel::Channel(Epoll *ep, int fd) : epoll_(ep), fd_(fd)
{
}

Channel::~Channel()
{
    // 在析构函数中，不要销毁ep_，也不能关闭fd_，因为这两个东西不属于Channel类，Channel类只是需要它们，使用它们而已。
}

int Channel::fd()
{
    return fd_;
}

void Channel::useet()
{
    events_ |= EPOLLET; // 采用边缘触发。
    // events_ = events_ | EPOLLET; // 采用边缘触发。
}

void Channel::enablereading()
{
    events_ |= EPOLLIN;          // 让epoll_wait()监视fd_的读事件。
    epoll_->updateChannel(this); // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
}

void Channel::setinepoll()
{
    inepoll_ = true;
}

void Channel::setrevents(uint32_t ev)
{
    revents_ = ev;
}

bool Channel::getinpoll()
{
    return inepoll_;
}

uint32_t Channel::getevents()
{
    return events_;
}

uint32_t Channel::getrevents()
{
    return revents_;
}