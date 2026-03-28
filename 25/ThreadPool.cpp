#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadnum):stop_(false) 
{
    // 启动threadnum个线程，每个线程将阻塞在条件变量上。
	for (size_t ii = 0; ii < threadnum; ii++)
    {
        /*  两种创建方式 ：其中[]{}代表lambda函数
            std::thread tt([]{});
            threads_.push_back(tt);
            threads_.emplace_back([]{});
        */
        // 用lambda函创建线程。
		threads_.emplace_back([this]
        {
            printf("create thread(%ld).\n",syscall(SYS_gettid));     // 显示线程ID。
            // std::cout << "子线程：" << std::this_thread::get_id() << std::endl; // 这个显示的ID比较难用

			while (stop_==false)
			{
				std::function<void()> task;       // 用于存放出队的元素。
                // 锁作用域结束后，互斥锁自动释放。也可以用unlock()释放
				{   // 锁作用域的开始。 ///////////////////////////////////
					std::unique_lock<std::mutex> lock(this->mutex_);

					// 等待生产者的条件变量。第二参数是谓词，用于处理虚假唤醒。循环的阻塞当前线程，直到通知到达且谓词满足
					this->condition_.wait(lock, [this] 
                    {
                        return ((this->stop_==true) || (this->taskqueue_.empty()==false));
                    }); // 也能这么写：auto f = []{}; wait(lock, f);

                    // 在线程池停止之前，如果队列中还有任务，执行完再退出。
					if ((this->stop_==true)&&(this->taskqueue_.empty()==true)) return;

                    // 出队一个任务。
					task = std::move(this->taskqueue_.front()); // 移动语义，避免拷贝开销
					this->taskqueue_.pop();
				}   // 锁作用域的结束。 ///////////////////////////////////
                // lock.unlock(); // 手动解锁

                printf("thread is %ld.\n",syscall(SYS_gettid));
				task();  // 执行任务。
			}
		});
    }
}

void ThreadPool::addtask(std::function<void()> task)
{
    // 加锁、将任务放入循环队列，出作用域自动解锁。
    {   // 锁作用域的开始。 ///////////////////////////////////
        std::lock_guard<std::mutex> lock(mutex_);   
        taskqueue_.push(task);  
    }   // 锁作用域的结束。 ///////////////////////////////////

    condition_.notify_one();   // 唤醒一个线程。
    // condition_.notify_all();   // 唤醒所有线程
}

ThreadPool::~ThreadPool()
{
	stop_ = true;

	condition_.notify_all();  // 唤醒全部的线程。

    // 等待全部线程执行完任务后退出。
	for (std::thread &th : threads_) 
        th.join();
}

void show(int no, const std::string &name)
{
    printf("小哥哥们好，我是第%d号超级女生%s。\n",no,name.c_str());
}

void test()
{
    printf("我有一只小小鸟。\n");
}

int main()
{
    ThreadPool threadpool(3);
    
    std::string name="西施";
    threadpool.addtask(std::bind(show,8,name));
    sleep(1);

    threadpool.addtask(std::bind(test));
    sleep(1);

    threadpool.addtask(std::bind([]{ printf("我是一只傻傻鸟。\n");}));
    sleep(1);
}

// g++ -o test ThreadPool.cpp -lpthread