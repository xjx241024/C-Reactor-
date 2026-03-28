#include "EventLoop.h"

EventLoop::EventLoop() : epoll_(new Epoll()), wakeUpfd_(eventfd(0, EFD_NONBLOCK)), wakeChannel_(new Channel(this, wakeUpfd_))
{
    wakeChannel_->setReadCallback(std::bind(&EventLoop::handleWakeUp, this));
    wakeChannel_->enablereading();  // 注册读事件。只要事件循环被唤醒，就会调用handleWakeUp()
}

EventLoop::~EventLoop()
{
    // delete epoll_;
}

void EventLoop::run()
{
    threadID_ = syscall(SYS_gettid); // 获取线程ID
    // printf("EventLoop::run() thread is %ld.\n",syscall(SYS_gettid));     // 显示线程ID。
    while (true) // 事件循环。
    {
        // std::vector<Channel *> channels = epoll_->loop(); // 存放epoll_wait()返回的事件,每个事件对应一个Channel对象，Channel对象中有fd和需要监视的事件。
        std::vector<Channel *> channels = epoll_->loop(10 * 1000); // 测试超时情况
        if (channels.size() == 0)
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

Epoll *EventLoop::getEpoll() // 主要返回的是对象指针，要用*来声明返回值类型
{
    return epoll_.get(); // 获取epoll对象指针
}

void EventLoop::updateChannel(Channel *ch)
{
    epoll_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel *ch)
{
    epoll_->removeChannel(ch); // 调用Epoll类的对应函数
}

void EventLoop::setEpollTimeoutCallback(std::function<void(EventLoop *)> cb)
{
    epollTimeoutCallback_ = cb;
}

bool EventLoop::isLoopThread()
{
    return threadID_ == syscall(SYS_gettid);
}

void EventLoop::queueInLoop(std::function<void()> fn)
{
    // 锁的作用域
    {
        std::lock_guard<std::mutex> gd(mutex_); // 给任务队列加锁
        taskQueue_.push(fn);                    // 任务入队
    }

    wakeUp(); // 唤醒事件循环
}

void EventLoop::wakeUp()
{
    uint64_t val = 1;
    write(wakeUpfd_, &val, sizeof(val)); // 随便写一点数据即可，不关心写什么，因为有写入就可以唤醒线程
}

void EventLoop::handleWakeUp()
{
    printf("handleWakeUp() thread ID is %ld\n", syscall(SYS_gettid)); // 日志,方便调试

    uint64_t val;
    read(wakeUpfd_, &val, sizeof(val));

    std::function<void()> fn;

    std::lock_guard<std::mutex> gd(mutex_);

    while (taskQueue_.size())
    {
        fn = std::move(taskQueue_.front()); // 获取任务
        taskQueue_.pop();                   // 移动语义后的元素以及没法使用，将其出队
        fn();                               // 执行任务
    }
}