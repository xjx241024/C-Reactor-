#pragma once
#include "Epoll.h"
#include "Connection.h"
#include <functional>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h> // 定时器
#include <unistd.h>
#include <memory>
#include <queue>
#include <mutex>
#include <map>
#include <atomic>

class Epoll;   // 前向声明，告诉编译器Epoll是一个类，编译器就知道Epoll*是一个指针类型了，就不会报错了
class Channel; // 前向声明，告诉编译器Channel是一个类，编译器就知道Channel*是一个指针类型了，就不会报错了
class Connection;

using spConnection = std::shared_ptr<Connection>;

class EventLoop
{
private:
    int timeInterval_; // 闹钟时间间隔，单位：秒。。
    int timeOut_;      // Connection对象超时的时间，单位：秒。

    std::unique_ptr<Epoll> epoll_; // epoll对象指针
    //  每个事件循环只有一个epoll对象，EventLoop对象中有一个Epoll对象指针成员

    pid_t threadID_;                              // 事件循环所在的线程ID
    std::queue<std::function<void()>> taskQueue_; // 事件循环线程被eventfd唤醒后执行的任务队列
    std::mutex mutex_;                            // 任务队列同步的互斥锁
    int wakeUpfd_;                                // 用于唤醒事件循环线程的fd
    std::unique_ptr<Channel> wakeChannel_;        // eventfd的Channel
    int timerfd_;                                 // 定时器的fd
    std::unique_ptr<Channel> timerChannel_;       // 定时器fd的Channel

    bool isMainLoop_ = true; // true表示主事件循环，false表示从事件循环

    std::map<int, spConnection> conns_; // map<fd, conn>，存放在该事件循环上的全部conn

    std::function<void(EventLoop *)> epollTimeoutCallback_; // epoll_wait()超时回调函数。
    std::function<void(int)> timerCallback_;

    std::atomic_bool isStop_; // true表示停止事件循环，其余线程可能调用stop()函数，所以用原子类型

public:
    EventLoop(bool isMainLoop, int timeInterval = 30, int timeOut = 80);
    ~EventLoop();
    void run();        // 事件循环
    void stop();       // 退出事件循环
    Epoll *getEpoll(); // 获取epoll对象指针

    void updateChannel(Channel *ch); // 调用Epoll类的updateChannel()函数
    void removeChannel(Channel *ch); // 从红黑树上删除Channel
    void setEpollTimeoutCallback(std::function<void(EventLoop *)> cb);

    bool isLoopThread(); // 判断线程是否是事件循环所在的线程

    void queueInLoop(std::function<void()> fn); // 把任务添加到队列中
    void wakeUp();                              // 唤醒事件循环
    void handleWakeUp();                        // 事件循环被唤醒后执行

    void handleTimer(); // 处理定时器事件

    void newConnection(spConnection conn); // 将conn保存在conns_

    void setTimerCallback(std::function<void(int)> cb);
};