#include "extra.h"
#include "malloc.h"
#include <Windows.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

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

char* copy_string(const char* string)
{
    int n = strlen(string);
    char* copied = st_malloc((n+1) * sizeof(char));
    strncpy(copied, string, n+1);
    copied[n] = '\0';
    return copied;
}
