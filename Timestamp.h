#pragma once

#include<iostream>
#include<string>

namespace lg{

class Timestamp
{
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondSinceEpoch);//防止隐形转换

    static Timestamp now();
    std::string toString() const;
private:
    int64_t microSecondSinceEpoch_;
};
}