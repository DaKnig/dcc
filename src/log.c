#include "log.h"

#include "util.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void log_printf(int type, const char *fmt, ...) {
    switch (type) {
    case LOG_INFO: break;
    case LOG_WARN:
        fprintf(stderr, "[" ANSI_COLOR("warning", YELLOW) "] ");
        break;
    case LOG_ERROR: fprintf(stderr, "[" ANSI_COLOR(" error ", RED) "] "); break;
    case LOG_FATAL: fprintf(stderr, "[" ANSI_COLOR(" fatal ", RED) "] "); break;
    default: assert(!"bad log type");
    }

    const size_t len = strlen(fmt);
    FILE *const sink = type == LOG_INFO ? stdout : stderr;

    va_list ap;
    va_start(ap, fmt);
    vfprintf(sink, fmt, ap);
    va_end(ap);

    if (len && fmt[len - 1] == ':') { fprintf(sink, " %s\n", strerror(errno)); }

    if (type != LOG_INFO) fflush(sink);

    if (type == LOG_FATAL) { exit(EXIT_FAILURE); }
}
