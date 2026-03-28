#include "Connection.h"

// 用智能指针后，原本的赋值函数删除了，要使用移动语义
Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> socket)
           : loop_(loop), connectionSocket_(std::move(socket)), disconnect_(false), connectionChannel_(new Channel(loop_, connectionSocket_->Sockfd()))
{
    // connectionChannel_ = new Channel(loop_, connectionSocket_->Sockfd()); // 用新连接的fd来构造一个Channel对象。
    // clientChannel只能new出来，不能在栈上，否则析构函数会关闭fd
    connectionChannel_->setReadCallback(std::bind(&Connection::onMessage, this));      // 设置clientChannel的读事件回调函数为onMessage函数。
    connectionChannel_->setCloseCallback(std::bind(&Connection::closeCallback, this)); // 设置clientChannel的关闭事件回调函数为Connection类的closeCallback函数。
    connectionChannel_->setErrorCallback(std::bind(&Connection::errorCallback, this)); // 设置clientChannel的错误事件回调函数为Connection类的errorCallback函数。
    connectionChannel_->setWriteCallback(std::bind(&Connection::writeCallback, this)); // 设置clientChannel的写事件回调函数为writeCallback函数
    connectionChannel_->useet();                                                       // 设置边缘触发
    connectionChannel_->enablereading();                                               // 让epoll_wait()监视clientfd的读事件。
}

Connection::~Connection()
{
    // delete connectionChannel_;
    // delete connectionSocket_;
    // Channel中new了一个clientSock，但没有释放，由于clientSock的生命周期和Connection一样，所以在Connection的析构函数中释放它。
    std::cout << "conn已经被释放" << std::endl;
}

int Connection::fd() const
{
    return connectionSocket_->Sockfd();
}

const char *Connection::ip() const
{
    return connectionSocket_->ip();
}

uint16_t Connection::port() const
{
    return connectionSocket_->port();
}

void Connection::closeCallback()
{
    disconnect_ = true;
    connectionChannel_->removeChannel();    // 从事件循环中删除Channel
    // 使用了智能指针，所以用shared_from_this()代替this
    closeCallback_(shared_from_this()); // 连接关闭(断开)时的回调函数，调用它来处理连接关闭。
}

void Connection::errorCallback()
{
    disconnect_ = true;
    connectionChannel_->removeChannel();    // 从事件循环中删除Channel
    errorCallback_(shared_from_this()); // 连接发生错误时的回调函数，调用它来处理连接错误。
}

void Connection::setCloseCallback(std::function<void(spConnection)> cb)
{
    closeCallback_ = cb;
}

void Connection::setErrorCallback(std::function<void(spConnection)> cb)
{
    errorCallback_ = cb;
}

void Connection::setOnMessageCallback(std::function<void(spConnection, std::string &)> cb)
{
    onMessageCallback_ = cb;
}

void Connection::setSendCompleteCallback(std::function<void(spConnection)> cb)
{
    sendComplteCallback_ = cb;
}

void Connection::onMessage()
{
    char buffer[1024];
    while (true)
    {
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(fd(), buffer, sizeof(buffer)); // 这行代码用了read()，也可以用recv()，一样的，不要纠结。
        if (nread > 0)                                      // 成功的读取到了数据。
        {
            // // 把接收到的报文内容原封不动的发回去。
            printf("recv(eventfd=%d):%s\n", fd(), buffer);
            // send(fd(), buffer, strlen(buffer), 0);
            inputBuffer_.append(buffer, nread); // 把接收到的数据添加到输入缓冲区。
        }
        else if (nread == -1 && errno == EINTR) // 读取数据的时候被信号中断，继续读取。
        {
            continue;
        }
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) // 全部的数据已读取完毕。
        {
            while (true)
            {
                // 现在使用的是指定报文长度的协议，先读取报文头部，获取报文内容的长度，然后再读取报文内容。
                int len = 0;
                memcpy(&len, inputBuffer_.data(), 4); // 从输入缓冲区中读取报文头部。
                if (inputBuffer_.size() < len + 4)
                    break;
                std::string content(inputBuffer_.data() + 4, len); // 从输入缓冲区中读取报文内容，构造一个字符串对象。
                inputBuffer_.erase(0, 4 + len);                    // 从输入缓冲区中删除已经处理过的数据，删除的长度是报文头部的长度加上报文内容的长度。

                printf("message(eventfd=%d)%s\n", fd(), content.c_str());

                onMessageCallback_(shared_from_this(), content); // 调用处理消息的回调函数。
            }
            break;
        }
        else if (nread == 0) // 客户端连接已断开。
        {
            // printf("client(eventfd=%d) disconnected.\n", fd());
            // close(fd()); // 关闭客户端的fd。
            closeCallback(); // 连接关闭(断开)时的回调函数，调用它来处理连接关闭。
            break;
        }
    }
}

void Connection::send(const char *msg, size_t len)
{
    if(disconnect_ == true)
    {
        printf("客户端连接已断开, 不发送数据\n");
        return;
    }

    std::shared_ptr<std::string> message(new std::string(msg));


    if(loop_->isLoopThread())   // 判断当前线程是否是事件循环线程(IO线程)
    {
        printf("send()在IO线程中执行\n");
        sendInLoop(message);
    }
    else
    {
        // 如果当前线程不是IO线程，交给IO线程发送
        printf("send()不在IO线程中执行\n");
        // 调用EventLoop::queueInLoop()，让事件循环线程发送
        loop_->queueInLoop(std::bind(&Connection::sendInLoop, this, message));
    }

}

/* 
void Connection::sendInLoop(const char *msg, size_t len)
{
    outputBuffer_.appendMessage(msg, len);      // 把要发送的数据（报文头部+报文内容）添加到输出缓冲区。
    connectionChannel_->enablewriting(); // 注册写事件
    // 一旦socket可写立刻发送数据。
} 
*/

void Connection::sendInLoop(std::shared_ptr<std::string> msg)
{
    outputBuffer_.appendMessage(msg->data(), msg->size());  // 把需要发送的数据放入发送缓冲区
    connectionChannel_->enablewriting();    // 注册写事件
}

void Connection::writeCallback()
{                                                                             // 需要循环来处理没能全部发送的情况？
    int writen = ::send(fd(), outputBuffer_.data(), outputBuffer_.size(), 0); // 尝试将发送缓冲区的数据全部发送。
    if (writen > 0)
        outputBuffer_.erase(0, writen); // 成功则清除发送缓冲区

    // 发送缓冲区没有数据，表示已经发送成功了，不再关注写事件了。
    if (outputBuffer_.size() == 0)
    {
        connectionChannel_->disablewriting();
        sendComplteCallback_(shared_from_this());
    }
}