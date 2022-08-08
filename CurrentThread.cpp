
#include "CurrentThread.h"

using namespace lg::CurrentThread;


void lg::CurrentThread::cacheTid()
{
    if(t_cachedTid == 0)
    {
        t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
    }
}