#include"loger.h"
#include"Timestamp.h"

#include<iostream>
using namespace lg;

Logger& Logger::getinstance()
{
    static Logger logger;
    return logger;
}
//设置日志等级
void Logger::setloglevel(int loglevel)
{
    loglevel_ = loglevel;
}
//写日志 格式：[级别信息] 时间：信息
void Logger::log(std::string message)
{
    switch (loglevel_)
    {
    case INFO:
        std::cout<<"[INFO]";
        break;
    case ERROR:
        std::cout<<"[ERROR]";
        break;

    case FATAL:
        std::cout<<"[FATAL]";
        break;
    case DEBUG:
        std::cout<<"[DEBUG]";
        break;    
    default:
        break;
    }
    //打印时间和信息
    std::cout<<Timestamp::now().toString()<< ", "<< message<<std::endl;
}
