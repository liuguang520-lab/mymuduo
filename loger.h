#pragma once
#include "nocopyable.h"
#include<string>
namespace lg
{

//日志的等级
enum LogLevel
{
    INFO,//普通信息
    ERROR,//错误信息
    FATAL,//core错误
    DEBUG,// 调试信息
};

#define LOG_INFO(logformat, ...)\
    do\
    {\
        Logger& logger = Logger::getinstance();\
        logger.setloglevel(INFO);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logformat, ##__VA_ARGS__);\
        logger.log(buf);\
    } while (0);
    
#define LOG_ERROR(logformat, ...)\
    do\
    {\
        Logger& logger = Logger::getinstance();\
        logger.setloglevel(ERROR);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logformat, ##__VA_ARGS__);\
        logger.log(buf);\
    } while (0);

#define LOG_FATAL(logformat, ...)\
    do\
    {\
        Logger& logger = Logger::getinstance();\
        logger.setloglevel(FATAL);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logformat, ##__VA_ARGS__);\
        logger.log(buf);\
        exit(-1);\
    } while (0);

#ifdef WANTDEBUG//在运行的时候一般不吧debug信息放入
#define LOG_DEBUG(logformat, ...)\
    do\
    {\
        Logger& logger = Logger::getinstance();\
        logger.setloglevel(DEBUG);\
        char buf[1024] = {0};\
        snprintf(buf, 1024, logformat, ##__VA_ARGS__);\
        logger.log(buf);\
    } while (0);
#else
#define LOG_DEBUG(logformat, ...)
#endif

class Logger : nocopyable
{
public:
    //获取实例
    static Logger& getinstance();
    //设置日志等级
    void setloglevel(int loglevel);
    //写日志
    void log(std::string message);
private:
    int loglevel_;
    Logger(){}
};

}