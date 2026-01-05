#include "extra.h"
#include "malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <glfw.h>

#ifdef _WIN32
#include <windows.h>
void sleep(i32 msec)
{
    Sleep(msec);
}
#else
#define sleep sleep_orig
#include <unistd.h>
#undef sleep
void sleep(i32 msec)
{
    usleep(1000*msec);
}
#endif

f64 get_time(void)
{
    return glfwGetTime();
}

char* string_copy(const char* string)
{
    int n = strlen(string);
    char* copied = st_malloc((n+1) * sizeof(char));
    strncpy(copied, string, n+1);
    copied[n] = '\0';
    return copied;
}

char* string_create(const char* format, int n, ...)
{
    char* string = st_malloc((n+1) * sizeof(char));
    va_list args;
    va_start(args, n);
    vsnprintf(&string[0], n, format, args);
    va_end(args);
    i32 len = strlen(string);
    string = st_realloc(string, (len+1) * sizeof(char));
    return string;
}

void string_free(char* string)
{
    st_free(string);
}

i32 maxi(i32 x, i32 y)
{
    return (x > y) ? x : y;
}

i32 mini(i32 x, i32 y)
{
    return (x < y) ? x : y;
}

f32 maxf(f32 x, f32 y)
{
    return (x > y) ? x : y;
}

f32 minf(f32 x, f32 y)
{
    return (x < y) ? x : y;
}
