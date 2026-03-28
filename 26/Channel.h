#pragma once
#include <sys/epoll.h>
#include <functional>
#include "Epoll.h"
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"

class EventLoop;
// class Epoll;

class Channel
{
private:
    int fd_; // Channel拥有的fd，Channel和fd是一对一的关系。
    // Epoll *epoll_ = nullptr; // Channel对应的红黑树，Channel与Epoll是多对一的关系，一个Channel只对应一个Epoll。
    EventLoop *eventloop_ = nullptr;      // Channel对应的事件循环，Channel与EventLoop是多对一的关系，一个Channel只对应一个EventLoop。
    bool inepoll_ = false;                // Channel是否已添加到epoll树上，如果未添加，调用epoll_ctl()的时候用EPOLL_CTL_ADD，否则用EPOLL_CTL_MOD。
    uint32_t events_ = 0;                 // fd_需要监视的事件。listenfd和clientfd需要监视EPOLLIN，clientfd还可能需要监视EPOLLOUT。
    uint32_t revents_ = 0;                // fd_已发生的事件。
    std::function<void()> readcallback_;  // 读事件的回调函数，类型为std::function<void()>，表示一个无参数、无返回值的函数对象。
    std::function<void()> writecallback_; // 写事件的回调函数，类型为std::function<void()>，表示一个无参数、无返回值的函数对象。
    std::function<void()> closecallback_; // 连接关闭(断开)时的回调函数，类型为std::function<void()>，表示一个无参数、无返回值的函数对象。
    std::function<void()> errorcallback_; // 连接发生错误时的回调函数，类型为std::function<void()>，表示一个无参数、无返回值的函数对象。

public:
    Channel(EventLoop *eventloop, int fd); // 构造函数。
    ~Channel();                            // 析构函数。

    int fd();                     // 返回fd_成员。
    void useet();                 // 采用边缘触发。

    void enablereading();         // 让epoll_wait()监视fd_的读事件。
    void disablereading();        // 让epoll_wait()不再监视fd_的读事件。
    void enablewriting();         // 让epoll_wait()监视fd_的写事件。
    void disablewriting();        // 让epoll_wait()不再监视fd_

    void setinepoll();            // 把inepoll_成员的值设置为true。
    void setrevents(uint32_t ev); // 设置revents_成员的值为参数ev。
    bool getinpoll();             // 返回inepoll_成员。
    uint32_t getevents();         // 返回events_成员。
    uint32_t getrevents();        // 返回revents_成员。
    void handleevents();          // 处理事件的函数，epoll_wait()返回时，执行它

    void setReadCallback(std::function<void()> cb);  // 设置读事件的回调函数。
    void setWriteCallback(std::function<void()> cb); // 设置写事件的回调函数。
    void setErrorCallback(std::function<void()> cb); // 设置错误事件的回调函数。
    void setCloseCallback(std::function<void()> cb); // 设置关闭事件的回调函数。
};