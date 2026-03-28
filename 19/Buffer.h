#pragma once
#include <string>
#include <iostream>

class Buffer
{
private:
    std::string buffer_; // string用于存放数据

public:
    Buffer();
    ~Buffer();

    void append(const char *str, size_t len); // 向缓冲区添加数据。添加只是写的一种，所以不命名为write
    void clear();                             // 清空缓冲区
    const char *data() const;                // 获取缓冲区中的数据
    size_t size() const;                      // 获取缓冲区中数据的大小
};