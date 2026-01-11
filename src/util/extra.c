#include "extra.h"
#include "malloc.h"
#include <stdio.h>
#include <math.h>
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
    i32 n = strlen(string);
    char* copied = st_malloc((n+1) * sizeof(char));
    strncpy(copied, string, n+1);
    copied[n] = '\0';
    return copied;
}

char* string_copy_len(const char* string, i32* len)
{
    i32 n = strlen(string);
    char* copied = st_malloc((n+1) * sizeof(char));
    strncpy(copied, string, n+1);
    copied[n] = '\0';
    *len = n;
    return copied;
}

char* string_create(const char* fmt, ...)
{
    i32 n;
    va_list args;
    char* string;
    va_start(args, fmt);
    n = vsnprintf(NULL, 0, fmt, args);
    string = st_malloc((n+1) * sizeof(char));
    vsnprintf(string, n+1, fmt, args);
    va_end(args);
    return string;
}

char* string_create_len(const char* fmt, i32* len, ...)
{
    i32 n;
    va_list args;
    char* string;
    va_start(args, len);
    n = vsnprintf(NULL, 0, fmt, args);
    string = st_malloc((n+1) * sizeof(char));
    vsnprintf(string, n+1, fmt, args);
    va_end(args);
    *len = n;
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

f32 lerp(f32 low, f32 high, f32 max_t, f32 t)
{
    if (t < 0) return low;
    if (t > max_t) return high;
    return low + (high - low) * t / max_t;
}

f32 randf(void)
{
    return (f32)(rand()) / RAND_MAX;
}

f32 randf_range(f32 low, f32 high)
{
    return (f32)(rand()) / RAND_MAX * (high-low) + low;
}

f32 guass_dist(f32 mean, f32 std)
{
    f32 u1, u2, z;
    u1 = randf() + EPSILON;
    u2 = randf();
    z = sqrt(-2 * log(u1)) * cos(2 * PI * u2);
    return z * std + mean;
}

f64 gmodf(f64 x, f64 m)
{
    return x - m * floor(x / m);
}
