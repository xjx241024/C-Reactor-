#include "Buffer.h"

Buffer::Buffer(uint16_t seq) : seq_(seq)
{
}

Buffer::~Buffer()
{
}

void Buffer::append(const char *str, size_t len)
{
    buffer_.append(str, len); // 将输入的字符串添加到缓冲区
}

void Buffer::appendWithSeq(const char *str, size_t len)
{
    if (seq_ == 0) // 无分隔符
    {
        buffer_.append(str, len); // 添加报文内容
    }
    else if (seq_ == 1) // 四字节的报头
    {
        buffer_.append((char *)&len, 4); // 将4字节的整数（报文头部）添加进去。
        buffer_.append(str, len);        // 添加报文内容
    }
    else if (seq_ == 2) // "\r\n\r\n"分隔符
    {
        buffer_.append(str, len);      // 添加报文内容
        buffer_.append("\r\n\r\n", 4); // 添加HTTP分隔符
    }
}

void Buffer::erase(const size_t pos, const size_t len)
{
    buffer_.erase(pos, len); // 从缓冲区中删除数据，从pos位置开始删除len长度的数据
}

void Buffer::clear()
{
    buffer_.clear(); // 清空缓冲区
}

const char *Buffer::data() const
{
    return buffer_.data(); // 返回缓冲区中的数据
}

size_t Buffer::size() const
{
    return buffer_.size(); // 返回缓冲区中数据的大小
}

bool Buffer::pickMessage(std::string &str)
{
    if (!buffer_.size())
        return false;

    if (seq_ == 0)
    {
        str = buffer_;
        buffer_.clear();
    }
    else if (seq_ == 1)
    {
        int len = 0;
        memcpy(&len, buffer_.data(), 4); // 从buffer_中读取报文头部。
        if (buffer_.size() < len + 4)
            return false;

        // str = std::string(buffer_.data() + 4, len); // 从buffer_读取报文内容，构造一个字符串对象。
        str = buffer_.substr(4, len);   // 从字符串中提取子字符串
        buffer_.erase(0, 4 + len);                  // 从buffer_中删除已经处理过的数据，删除的长度是报文头部的长度加上报文内容的长度。
    }
    else if (seq_ == 2)
    {
        size_t pos = buffer_.find("\r\n\r\n");
        if (pos == std::string::npos)   // std::string::npos 是一个静态常量,用于表示 find、rfind 等函数在字符串中未找到子串时的返回值
            return false;

        str = buffer_.substr(0, pos);
        buffer_.erase(0, pos + 4);
    }

    return true;
}