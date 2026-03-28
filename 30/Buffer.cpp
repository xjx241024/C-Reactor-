#include "Buffer.h"

Buffer::Buffer() : buffer_("") 
{

}

Buffer::~Buffer() 
{

}

void Buffer::append(const char *str, size_t len) 
{
    buffer_.append(str, len); // 将输入的字符串添加到缓冲区
}

void Buffer::appendMessage(const char *str, size_t len)
{
    buffer_.append((char*)&len, 4); // 将4字节的整数（报文头部）添加进去。
    buffer_.append(str, len);   // 添加报文内容
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