// config.cc calls nh_log() for diagnostic messages. The real implementation
// (NickelHook/nh.c) writes to syslog and is part of the Kobo cross-compiled
// build; this stub satisfies the link for host-side tests.

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
