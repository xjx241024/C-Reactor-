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

    void append(const char *str, size_t len); // 向缓冲区添加数据。
    void erase(const size_t pos, const size_t len);  // 从缓冲区中删除数据。从pos位置开始删除len长度的数据。
    void clear();                             // 清空缓冲区
    const char *data() const;                 // 获取缓冲区中的数据
    size_t size() const;                      // 获取缓冲区中数据的大小
};