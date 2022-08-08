#pragma once

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size

#include<vector>

#include"nocopyable.h"

#include<algorithm>
#include<string>

namespace lg
{

class Buffer : public nocopyable
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        :   buffer_(kCheapPrepend + kInitialSize),
            readerIndex_(kCheapPrepend),
            writeIndex_(kCheapPrepend)
        {}//读写头一开始在一起表示没有数据可读，前面的标识是为了能解决粘包问题

    size_t readableBytes() const
    {
        return writeIndex_ - readerIndex_;
    }
    size_t writeableBytes()const
    {
        return buffer_.size() - writeIndex_; 
    }
    size_t prependableBytes()const
    {
        return readerIndex_;
    }

    //返回缓冲区中可读缓冲区的起始地址
    const char* peek()const
    {
        return begin() + readerIndex_;
    }


    //调整读写下标
    void retrive(size_t len)
    {
        if(len < readableBytes())
        {
            readerIndex_ += len;//表示数据没有从缓冲区读完
        }
        else
        {
            retriveAll();
        }
    }

    void retriveAll()
    {
        readerIndex_ = kCheapPrepend;
        writeIndex_ = kCheapPrepend;
    }


    //将缓冲区的数据读完并且转换成string
    std::string retriveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(begin(), len);
        retrive(len);
        return result;
    }

    void ensureWritableBytes(size_t len)
    {
        if(writeableBytes() < len)
        {
            makeSpace(len);
        }
    }

    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    } 


    char* beginWrite()
    {
        return begin() + writeIndex_;
    }
    void hasWritten(size_t len)
    {
        writeIndex_ += len;
    }

    ssize_t readFD(int fd, int* saveErrno);
    ssize_t writeFD(int fd, int* saveErrno);
private:

    char* begin()
    {
        return &*buffer_.begin();
    }
    const char* begin() const
    {
        return &*buffer_.begin();
    }


    //将还没有读取的数据复制到前面已经读了的数据区
    void makeSpace(size_t len)
    {
        if(writeableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writeIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin()+readerIndex_, begin() + writeIndex_, begin()+kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writeIndex_ = readerIndex_ + readable;
        }
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writeIndex_;
};

}