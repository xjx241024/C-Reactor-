#include "TCPServer.h"

TCPServer::TCPServer(const char *ip, uint16_t port, int threadNum, uint16_t seq)
    : seq_(seq), threadNum_(threadNum), acceptor_(&mainLoop_, ip, port), threadPool_(threadNum_, "IO"), mainLoop_(true)
{
    mainLoop_.setEpollTimeoutCallback(std::bind(&TCPServer::epollTimeout, this, std::placeholders::_1));

    // acceptor_ = new Acceptor(&mainLoop_, ip, port);                                                         // 在构造函数中创建一个EventLoop对象，并将其地址赋值给这个指针成员。
    acceptor_.setNewConnectionCallback(std::bind(&TCPServer::newConnection, this, std::placeholders::_1)); // 设置处理新连接的回调函数

    // threadPool_ = new ThreadPool(threadNum_, "IO"); // 创建线程池

    // 创建从事件的循环
    for (int i = 0; i < threadNum_; i++)
    {
        subLoops_.push_back(std::make_unique<EventLoop>(false, 5, 10)); // 两种写法
        // subLoops_.emplace_back(new EventLoop(false, 5, 10));  // 创建从事件并放入容器中
        subLoops_[i]->setEpollTimeoutCallback(std::bind(&TCPServer::epollTimeout, this, std::placeholders::_1)); // 设置超时回调函数
        subLoops_[i]->setTimerCallback(std::bind(&TCPServer::removeTimeoutConn, this, std::placeholders::_1));   // 设置清理空闲连接的回调函数
        threadPool_.addtask(std::bind(&EventLoop::run, subLoops_[i].get()));                                     // 在线程池中运行事件循环,第二个参数要填普通指针
    }
}

TCPServer::~TCPServer()
{
    // delete acceptor_;

    /*  智能指针，会自动释放，所以这部分代码不需要了
    for (auto &item : connections_)
        delete item.second; // 释放所有Connection对象的内存。
    */

    // for (auto &item : subLoops_)
    //     delete item; // 释放所有从事件循环的内存。

    // delete threadPool_;
}

void TCPServer::start()
{
    mainLoop_.run(); // 运行事件循环，等待事件的发生，并处理事件。
}

void TCPServer::stop()
{
    mainLoop_.stop(); // 停止主事件循环

    for(int i = 0; i < threadNum_; i++)
    {
        subLoops_[i]->stop();// 停止从事件循环
    }

    threadPool_.stop();    // 停止IO线程
}

void TCPServer::onMessage(spConnection conn, std::string &message)
{
    if (onMessageCallback_)
        onMessageCallback_(conn, message); // 回调EchoServer::handleMessage
}

void TCPServer::newConnection(std::unique_ptr<Socket> clientSock)
{
    // 将新建的conn分配给从事件循环，两种方法避免移动语义后对clientSock->Sockfd()的操作：
    int fd = clientSock->Sockfd();
    spConnection conn(new Connection(subLoops_[fd % threadNum_].get(), std::move(clientSock), seq_)); // 智能指针对象创建
    // spConnection conn{new Connection{subLoops_[clientSock->Sockfd() % threadNum_].get(), std::move(clientSock)}};
    conn->setCloseCallback(std::bind(&TCPServer::closeConnection, this, std::placeholders::_1));                      // 设置连接关闭(断开)时的回调函数，参数是TCPServer对象的成员函数closeConnection，绑定this指针和占位符_1。
    conn->setErrorCallback(std::bind(&TCPServer::errorConnection, this, std::placeholders::_1));                      // 设置连接发生错误时的回调函数，参数是TCPServer对象的成员函数errorConnection，绑定this指针和占位符_1。
    conn->setOnMessageCallback(std::bind(&TCPServer::onMessage, this, std::placeholders::_1, std::placeholders::_2)); // 设置处理消息的回调函数，参数是TCPServer对象的成员函数onMessage，绑定this指针和占位符_1、_2。
    conn->setSendCompleteCallback(std::bind(&TCPServer::sendComplete, this, std::placeholders::_1));                  // 发送完成时的回调函数

    {
        std::lock_guard<std::mutex> gd(mutex_);
        connections_[conn->fd()] = conn; // 将新连接的文件描述符和Connection对象指针添加到connections_ map中。
    }
    
    // 两种写法，和上面同样原理
    subLoops_[fd % threadNum_]->newConnection(conn); // 将新连接的文件描述符和Connection对象指针添加到事件循环的map中。
    // subLoops_[conn->fd() % threadNum_]->newConnection(conn);

    if (newConnectionCallback_)
        newConnectionCallback_(conn); // 连接建立后再回调，回调EchoServer::handleNewConnection
}

void TCPServer::closeConnection(spConnection conn)
{
    if (closeConnectionCallback_)
        closeConnectionCallback_(conn); // 先回调，再关闭连接。回调EchoServer::handleCloseConnection

    // printf("client(eventfd=%d) closed.\n", conn->fd());
    // close(conn->fd()); // Connection对象的析构函数会delete其Socket成员变量，而Socket对象的析构函数会关闭fd，所以这里不需要再调用close()函数来关闭fd了。
    {
        std::lock_guard<std::mutex> gd(mutex_);
         connections_.erase(conn->fd()); // 从connections_ map中删除对应的Connection对象指针。
    }
    // delete conn;
}

void TCPServer::errorConnection(spConnection conn)
{
    if (errorConnectionCallback_)
        errorConnectionCallback_(conn); // 先回调，再关闭连接。回调EchoServer::handleErrorConnection

    // printf("client(eventfd=%d) error.\n", conn->fd());

    {
        std::lock_guard<std::mutex> gd(mutex_);
        connections_.erase(conn->fd());
    }

    // delete conn;
}

void TCPServer::sendComplete(spConnection conn)
{
    // printf("send completed\n");
    if (sendCompleteCallback_)
        sendCompleteCallback_(conn); // 回调EchoServer::handleSendComplete
    // 业务通过回调函数，在EchoServer完成
}

void TCPServer::epollTimeout(EventLoop *loop)
{
    // printf("epoll_wait() timeout.\n");
    if (epollTimeoutCallback_)       // 检查是否包含有效的可调用目标（是否赋值）
        epollTimeoutCallback_(loop); // 回调EchoServer::handleEpollTimeout
    // 业务通过回调函数，在EchoServer完成
}

void TCPServer::removeTimeoutConn(int fd)
{
    // printf("TcpServer::removeconn() thread is %ld.\n",syscall(SYS_gettid)); 
    {
         std::lock_guard<std::mutex> gd(mutex_);
        connections_.erase(fd);          // 从map中删除conn。
    }
}

void TCPServer::setOnMessageCallback(std::function<void(spConnection, std::string &message)> cb)
{
    onMessageCallback_ = cb;
}

void TCPServer::setSendCompleteCallback(std::function<void(spConnection)> cb)
{
    sendCompleteCallback_ = cb;
}

void TCPServer::setCloseConnectionCallback(std::function<void(spConnection)> cb)
{
    closeConnectionCallback_ = cb;
}

void TCPServer::setErrorConnectionCallback(std::function<void(spConnection)> cb)
{
    errorConnectionCallback_ = cb;
}

void TCPServer::setEpollTimeoutCallback(std::function<void(EventLoop *)> cb)
{
    epollTimeoutCallback_ = cb;
}

void TCPServer::setNewConnectionCallback(std::function<void(spConnection)> cb)
{
    newConnectionCallback_ = cb;
}