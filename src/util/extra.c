#include "extra.h"
#include <Windows.h>
#include <sys/time.h>

void sleep(i32 msec)
{
    Sleep(msec);
}

f64 get_time(void)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec + time.tv_usec*1e-6;
}