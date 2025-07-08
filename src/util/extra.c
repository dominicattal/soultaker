#include "extra.h"
#include "malloc.h"
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <glfw.h>

void sleep(i32 msec)
{
    Sleep(msec);
}

f64 get_time(void)
{
    return glfwGetTime();
}

char* copy_string(const char* string)
{
    int n = strlen(string);
    char* copied = st_malloc((n+1) * sizeof(char));
    strncpy(copied, string, n+1);
    copied[n] = '\0';
    return copied;
}
