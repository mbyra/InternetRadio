#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstdarg>
#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include "err.h"

void syserr(const char *fmt, ...)
{
    va_list fmt_args;
    int err = errno;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);
    fprintf(stderr," (%d; %s)\n", err, strerror(err));
    exit(EXIT_FAILURE);
}

void fatal(const char *fmt, ...)
{
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);

    fprintf(stderr,"\n");
    exit(EXIT_FAILURE);
}


void debug(const char *fmt, ...)
{
    va_list fmt_args;

    fprintf(stderr, "DEBUG: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);

    fprintf(stderr,"\n");
//    exit(EXIT_FAILURE);
}
