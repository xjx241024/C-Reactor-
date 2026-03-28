#include <time.h>

#include "Timestamp.h"

Timestamp::Timestamp()
{
    secsinceepoch_=time(0);          // 取系统当前时间。
}

Timestamp::Timestamp(int64_t secsinceepoch): secsinceepoch_(secsinceepoch) 
{

}

// 当前时间。
Timestamp Timestamp::now() 
{ 
    return Timestamp();   // 返回当前时间。
}

time_t Timestamp::toint() const
{
    return secsinceepoch_;
}

#include <sstream>
#include <iomanip> 
// C++11 写法，无需手动计算大小，不会产生 -Wformat-truncation 警告
std::string Timestamp::tostring() const
{
    tm *tm_time = localtime(&secsinceepoch_);
    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << (tm_time->tm_year + 1900) << "-"
        << std::setw(2) << std::setfill('0') << (tm_time->tm_mon + 1) << "-"
        << std::setw(2) << std::setfill('0') << tm_time->tm_mday << " "
        << std::setw(2) << std::setfill('0') << tm_time->tm_hour << ":"
        << std::setw(2) << std::setfill('0') << tm_time->tm_min << ":"
        << std::setw(2) << std::setfill('0') << tm_time->tm_sec;
    return oss.str();
}

/* 
#include <unistd.h>
#include <iostream>

int main() 
{
    Timestamp ts;
    std::cout << ts.toint() << std::endl;
    std::cout << ts.tostring() << std::endl;

    sleep(1);
    std::cout << Timestamp::now().toint() << std::endl; // 利用静态成员函数，创建当前时间戳对象
    std::cout << Timestamp::now().tostring() << std::endl;
}

// g++ -o test Timestamp.cpp
 */