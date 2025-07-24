#include "extra.h"
#include "malloc.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <glfw.h>

void sleep(i32 msec)
{
    Sleep(msec);
}

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

char* string_create(const char* format, ...)
{
    static char string[STRING_CREATE_MAX_LENGTH+1];
    va_list args;
    va_start(args, format);
    vsnprintf(&string[0], STRING_CREATE_MAX_LENGTH, format, args);
    va_end(args);
    return string_copy(string);
}

void string_free(char* string)
{
    st_free(string);
}
