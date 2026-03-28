#include "EventLoop.h"

EventLoop::EventLoop() : epoll_(new Epoll()) 
{

}

EventLoop::~EventLoop() 
{
    delete epoll_;
}

void EventLoop::run() 
{
    std::vector<Channel *> channels; // 存放epoll_wait()返回的事件,每个事件对应一个Channel对象，Channel对象中有fd和需要监视的事件。

    while (true) // 事件循环。
    {
        channels = epoll_->loop(); // 运行epoll_wait()，等待事件的发生，已发生的事件用vector容器返回。

        // 如果infds>0，表示有事件发生的fd的数量。
        for (auto &ch : channels) // 遍历epoll返回的事件
        {
            ch->handleevents(); // 处理事件的函数
        }
    }
}

Epoll *EventLoop::getEpoll()    // 主要返回的是对象指针，要用*来声明返回值类型
{ 
    return epoll_;  // 获取epoll对象指针
}

void EventLoop::updateChannel(Channel *ch)
{
    epoll_->updateChannel(ch);     
}