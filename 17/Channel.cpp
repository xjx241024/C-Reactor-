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
    events_ |= EPOLLIN;          // 让epoll_wait()监视fd_的读事件。
    eventloop_->updateChannel(this); // 把channel添加/更新到红黑树上，channel中有fd，也有需要监视的事件。
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

void Channel::handleevents()
{
    if (revents_ & EPOLLRDHUP) // 对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0。
    {
        closecallback_(); // 连接关闭(断开)时的回调函数，调用它来处理连接关闭
    } //  普通数据  带外数据
    else if (revents_ & (EPOLLIN | EPOLLPRI)) // 接收缓冲区中有数据可以读。
    {
       // islisten_删除，直接调用读事件的回调函数，不需要判断是listenfd还是clientfd了，因为在设置回调函数的时候就已经区分开了。
       readcallback_();
    }
    else if (revents_ & EPOLLOUT) // 有数据需要写，暂时没有代码，以后再说。
    {
    }
    else // 其它事件，都视为错误。
    {
        errorcallback_(); // 连接发生错误时的回调函数，调用它来处理连接错误。
    }
}

void Channel::onMessage()
{
    char buffer[1024];
    while (true) // 由于使用非阻塞IO，一次读取buffer大小数据，直到全部的数据读取完毕。
    {
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(fd_, buffer, sizeof(buffer)); // 这行代码用了read()，也可以用recv()，一样的，不要纠结。
        if (nread > 0)                                     // 成功的读取到了数据。
        {
            // 把接收到的报文内容原封不动的发回去。
            printf("recv(eventfd=%d):%s\n", fd_, buffer);
            send(fd_, buffer, strlen(buffer), 0);
        }
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {
            continue;
        }
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            break;
        }
        else if (nread == 0) // 客户端连接已断开。
        {
            printf("client(eventfd=%d) disconnected.\n", fd_);
            close(fd_); // 关闭客户端的fd。
            break;
        }
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