#include "Channel.h"

Channel::Channel(EventLoop* eventloop, int fd) : eventloop_(eventloop), fd_(fd)
{
}

Channel::~Channel()
{
    // 在析构函数中，不要销毁eventloop_，也不能关闭fd_，因为这两个东西不属于Channel类，Channel类只是需要它们，使用它们而已。
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
    events_ |= EPOLLIN;          // 注册读事件
    eventloop_->updateChannel(this); // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
}

void Channel::disablereading()
{
    events_ &= ~EPOLLIN;         // 取消读事件
    eventloop_->updateChannel(this);
}

void Channel::enablewriting()
{
    events_ |= EPOLLOUT;         // 注册写事件。
    eventloop_->updateChannel(this);
}

void Channel::disablewriting()
{
    events_ &= ~EPOLLOUT;        // 取消写事件。
    eventloop_->updateChannel(this);
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

// 处理事件的函数，epoll_wait()返回时，执行它
void Channel::handleevents()
{
    if (revents_ & EPOLLRDHUP) // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        // printf("EPOLLRDHUP\n");
        closecallback_(); // 连接关闭(断开)时的回调函数，调用它来处理连接关闭
    } //  普通数据  带外数据
    else if (revents_ & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
    {
        // printf("EPOLLIN | EPOLLPRI\n");
        // islisten_删除，直接调用读事件的回调函数，不需要判断是listenfd还是clientfd了，因为在设置回调函数的时候就已经区分开了。
        readcallback_();
    }
    else if (revents_ & EPOLLOUT) // 有数据需要写
    {
        // printf("EPOLLOUT\n");
        writecallback_();
    }
    else // 其它事件，都视为错误。
    {
        // printf("ERROR\n");
        errorcallback_(); // 连接发生错误时的回调函数，调用它来处理连接错误。
    }
}

void Channel::setReadCallback(std::function<void()> cb)
{
    readcallback_ = cb;
}

void Channel::setWriteCallback(std::function<void()> cb)
{
    writecallback_ = cb;
}

void Channel::setCloseCallback(std::function<void()> cb)
{
    closecallback_ = cb;
}

void Channel::setErrorCallback(std::function<void()> cb)
{
    errorcallback_ = cb;
}