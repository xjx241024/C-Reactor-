#include "BankServer.h"

BankServer::BankServer(const char *ip, uint16_t port, int subThreadNum, int workThreadNum, uint16_t seq)
    : tcpServer_(ip, port, subThreadNum, seq), workThreadPool_(workThreadNum, "Work")
{
    // 设置回调函数
    tcpServer_.setNewConnectionCallback(std::bind(&BankServer::handleNewConnection, this, std::placeholders::_1));
    tcpServer_.setCloseConnectionCallback(std::bind(&BankServer::handleCloseConnection, this, std::placeholders::_1));
    tcpServer_.setErrorConnectionCallback(std::bind(&BankServer::handleErrorConnection, this, std::placeholders::_1));
    tcpServer_.setSendCompleteCallback(std::bind(&BankServer::handleSendComplete, this, std::placeholders::_1));
    // tcpServer_.setEpollTimeoutCallback(std::bind(&BankServer::handleEpollTimeout, this, std::placeholders::_1));
    tcpServer_.setOnMessageCallback(std::bind(&BankServer::handleMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcpServer_.setRemoveTimeoutConnCallback(std::bind(&BankServer::handleRemove, this, std::placeholders::_1));
}

BankServer::~BankServer()
{
}

void BankServer::start()
{
    tcpServer_.start();
}

void BankServer::stop()
{
    workThreadPool_.stop(); // 停止工作线程
    printf("工作线程已停止\n");

    tcpServer_.stop(); // 停止IO线程
    printf("IO线程已停止\n");
}

void BankServer::handleNewConnection(spConnection conn)
{
    printf("%s new connection(fd=%d,ip=%s,port=%d) ok.\n", Timestamp::now().toString().c_str(), conn->fd(), conn->ip(), conn->port());

    // 新客户端连上来时，把用户连接的信息保存到状态机中。
    spUserInfo userInfo = std::make_shared<UserInfo>(conn->fd(), conn->ip()); // 创建一个UserInfo对象，保存新连接的fd和ip地址。
    {
        std::lock_guard<std::mutex> lock(mutex_); // 保护userInfos_的互斥锁
        userInfos_[conn->fd()] = userInfo;         // 把这个UserInfo对象添加到userInfos_中，key是连接的fd。
    }

}

void BankServer::handleCloseConnection(spConnection conn)
{
    printf("%s close connection(fd=%d,ip=%s,port=%d) ok.\n", Timestamp::now().toString().c_str(), conn->fd(), conn->ip(), conn->port());

    // 从状态机中删除这个连接的信息。
    {
        std::lock_guard<std::mutex> lock(mutex_); // 保护userInfos_的互斥锁
        userInfos_.erase(conn->fd());              // 从userInfos_中删除这个连接的信息，key是连接的fd。
    }
}

void BankServer::handleErrorConnection(spConnection conn)
{
    printf("%s error connection(fd=%d,ip=%s,port=%d) ok.\n", Timestamp::now().toString().c_str(), conn->fd(), conn->ip(), conn->port());

    // 从状态机中删除这个连接的信息。
    {
        std::lock_guard<std::mutex> lock(mutex_); // 保护userInfos_的互斥锁
        userInfos_.erase(conn->fd());              // 从userInfos_中删除这个连接的信息，key是连接的fd。
    }
}

void BankServer::handleSendComplete(spConnection conn)
{
    // std::cout << "BankServer::handleSendComplete()运行： Message send complete." << std::endl;

    // 根据业务的需求，在这里可以增加其它的代码。
}

// void BankServer::handleEpollTimeout(EventLoop *loop)
// {
//     std::cout << "BankServer timeout." << std::endl;

//     // 根据业务的需求，在这里可以增加其它的代码。
// }

void BankServer::handleMessage(spConnection conn, std::string &message)
{

    if (workThreadPool_.size() == 0)
    {
        // 如果没有工作线程，在IO线程中计算
        onMessage(conn, message);
    }
    else
    {
        // 有工作线程，则在工作线程计算
        workThreadPool_.addtask(std::bind(&BankServer::onMessage, this, conn, message)); // 添加到工作线程池
    }
}

//处理xml报文
bool getxmlbuffer(const std::string &xmlbuffer,const std::string &fieldname,std::string  &value,const int ilen=0)
{
    std::string start="<"+fieldname+">";            // 数据项开始的标签。
    std::string end="</"+fieldname+">";            // 数据项结束的标签。

    int startp=xmlbuffer.find(start);                     // 在xml中查找数据项开始的标签的位置。
    if (startp==std::string::npos) return false;

    int endp=xmlbuffer.find(end);                       // 在xml中查找数据项结束的标签的位置。
    if (endp==std::string::npos) return false;

    // 从xml中截取数据项的内容。
    int itmplen=endp-startp-start.length();
    if ( (ilen>0) && (ilen<itmplen) ) itmplen=ilen;
    value=xmlbuffer.substr(startp+start.length(),itmplen);

    return true;
}

void BankServer::onMessage(spConnection conn, std::string &message)
{
    // printf("%s message(eventfd=%d)%s\n", Timestamp::now().toString().c_str(), conn->fd(), message.c_str());

    spUserInfo userInfo = userInfos_[conn->fd()]; // 从状态机中获取这个连接的信息，key是连接的fd。
    // 解析客户端报文请求
    // <bizcode>00101</bizcode><username>wucz</username><password>123465</password>
    std::string bizcode;
    std::string replaymessage;      // 回应报文。
    getxmlbuffer(message,"bizcode",bizcode);       // 从请求报文中解析出业务代码。

    if (bizcode=="00101")     // 登录业务。
    {
        std::string username,password;
        getxmlbuffer(message,"username",username);    // 解析用户名。
        getxmlbuffer(message,"password",password);     // 解析密码。
        if ( (username=="wucz") && (password=="123465") )         // 假设从数据库或Redis中查询用户名和密码。
        {
            // 用户名和密码正确。
            replaymessage="<bizcode>00102</bizcode><retcode>0</retcode><message>ok</message>";
            userInfo->setLogin(true);               // 设置用户的登录状态为true。
        }
        else
        {
            // 用户名和密码不正确。
            replaymessage="<bizcode>00102</bizcode><retcode>-1</retcode><message>用户名或密码不正确。</message>";
        }
    }
    else if (bizcode=="00201")   // 查询余额业务。
    {
        if (userInfo->Login()==true)
        {
            // 把用户的余额从数据库或Redis中查询出来。
            replaymessage="<bizcode>00202</bizcode><retcode>0</retcode><message>5088.80</message>";
        }
        else
        {
            replaymessage="<bizcode>00202</bizcode><retcode>-1</retcode><message>用户未登录。</message>";
        }
    }
    else if (bizcode=="00901")   // 注销业务。
    {
        if (userInfo->Login()==true)
        {
            replaymessage="<bizcode>00902</bizcode><retcode>0</retcode><message>ok</message>";
            userInfo->setLogin(false);               // 设置用户的登录状态为false。
        }
        else
        {
            replaymessage="<bizcode>00902</bizcode><retcode>-1</retcode><message>用户未登录。</message>";
        }
    }
    else if (bizcode=="00001")   // 心跳。
    {
        if (userInfo->Login()==true)
        {
            replaymessage="<bizcode>00002</bizcode><retcode>0</retcode><message>ok</message>";
        }
        else
        {
            replaymessage="<bizcode>00002</bizcode><retcode>-1</retcode><message>用户未登录。</message>";
        }
    }
    else
    {
        replaymessage="<bizcode>99999</bizcode><retcode>-1</retcode><message>业务代码不存在。</message>";
    }

    conn->send(replaymessage.data(),replaymessage.size());   // 把数据发送出去。 
}

void BankServer::handleRemove(int fd)
 {
    printf("fd(%d) 已超时。\n",fd);

    std::lock_guard<std::mutex> gd(mutex_);
    userInfos_.erase(fd);                                      // 从状态机中删除用户信息。
 }