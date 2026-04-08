#pragma once
#include <string>
#include <cstring>
#include <iostream>
#include <stdint.h>

class Buffer
{
private:
    std::string buffer_; // string用于存放数据

    // 报文的分隔符：0-无分隔符(固定长度、视频会议)；1-四字节的报头；2-"\r\n\r\n"分隔符（http协议）
    const uint16_t seq_; // 无符号 16 位整数类型

public:
    Buffer(uint16_t seq_ = 1);
    ~Buffer();

    void append(const char *str, size_t len);        // 向缓冲区添加数据。
    void appendWithSeq(const char *str, size_t len); // 把数据追加到buf_中，附加报文分隔符
    void erase(const size_t pos, const size_t len);  // 从缓冲区中删除数据。从pos位置开始删除len长度的数据。
    void clear();                                    // 清空缓冲区
    const char *data() const;                        // 获取缓冲区中的数据
    size_t size() const;                             // 获取缓冲区中数据的大小
    bool pickMessage(std::string &str);               // 从buf_中拆分出一个报文，存放在str中，如果buf_中没有报文，返回false。
};