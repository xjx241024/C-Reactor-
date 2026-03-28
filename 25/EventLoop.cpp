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
    while (true) // 事件循环。
    {
        //std::vector<Channel *> channels = epoll_->loop(); // 存放epoll_wait()返回的事件,每个事件对应一个Channel对象，Channel对象中有fd和需要监视的事件。
        std::vector<Channel *> channels = epoll_->loop(10*1000);// 测试超时情况
        if(channels.size() == 0)
        {
            epollTimeoutCallback_(this);
        }

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

void EventLoop::setEpollTimeoutCallback(std::function<void(EventLoop *)> cb)
{
    epollTimeoutCallback_ = cb;
}