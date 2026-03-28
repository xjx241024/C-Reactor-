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
        printf("client(eventfd=%d) disconnected.\n", fd_);
        close(fd_); // 关闭客户端的fd。
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
        printf("client(eventfd=%d) error.\n", fd_);
        close(fd_); // 关闭客户端的fd。
    }
}

void Channel::newConnection(Socket *serverSock)
{
    InetAddress clientaddr;                        // 客户端地址和协议信息
    int clientfd = serverSock->accept(clientaddr); // accept4()函数可以直接创建非阻塞的fd
    Socket *clientSock = new Socket(clientfd);     // 用accept4()函数返回的新连接的fd来构造一个Socket对象。
    // clientSock只能new出来，不能在栈上，否则析构函数会关闭fd
    // 如果不用new创建Socket对象，那么它的生命周期就和当前的作用域一样，出了作用域就会被销毁，这样就无法在事件循环中使用它了。

    // accept()中对clientaddr进行设置
    printf("accept client(fd=%d,ip=%s,port=%d) ok.\n", clientSock->Sockfd(), clientaddr.ip(), clientaddr.port());

    // Channel和Socket是多对一的关系，一个Channel对象对应一个fd，而一个fd可以有多个Channel对象监视不同的事件。
    // 这部分代码新建了客户端socket，所以需要新建一个Channel对象来监视这个客户端socket的事件。
    Channel *clientChannel = new Channel(eventloop_, clientSock->Sockfd()); // 用新连接的fd来构造一个Channel对象。
    // clientChannel只能new出来，不能在栈上，否则析构函数会关闭fd
    clientChannel->setReadCallback(std::bind(&Channel::onMessage, clientChannel)); // 设置clientChannel的读事件回调函数为onMessage函数。
    clientChannel->enablereading(); // 让epoll_wait()监视clientfd的读事件。
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
