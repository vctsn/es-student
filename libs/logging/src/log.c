#include "logging/log.h"
#include <stdarg.h>
#include <stdio.h>

void log_print(const char *level, const char *func, int line, const char *fmt, ...)
{
    printf("%s %s:%d ", level, func, line);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\r\n");
}
