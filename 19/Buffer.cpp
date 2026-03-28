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