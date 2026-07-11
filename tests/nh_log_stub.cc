#include <NickelHook.h>
#include <cstdarg>
#include <cstdio>

void nh_log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}
