#ifndef SHREDIS_COMMON_H
#define SHREDIS_COMMON_H

#include <cstdio>
#include <cstdarg>

#define SHREDIS_DEBUG true

__attribute__((format(printf, 1, 2)))
static void debug(const char* format, ...) {
#ifdef SHREDIS_DEBUG
    va_list arglist;
    va_start(arglist, format);
    vprintf(format, arglist);
    va_end(arglist);
#endif
}

#endif //SHREDIS_COMMON_H
