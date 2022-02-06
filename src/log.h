#ifndef LOG_H_
#define LOG_H_

#define STRX(x)         #x
#define STR(x)          STRX(x)
#define log_xxx(t, ...) log_printf(t, __VA_ARGS__)
#define log_print(...)  log_printf(LOG_INFO, __VA_ARGS__)
#define log_warn(...)   log_printf(LOG_WARN, __VA_ARGS__)
#define log_error(...)  log_printf(LOG_ERROR, __VA_ARGS__)
#define log_fatal(...)  log_printf(LOG_FATAL, __VA_ARGS__)
#define log_errorwloc(...) \
    log_printf(LOG_ERROR,  \
               ANSI_COLOR(__FILE__ ":" STR(__LINE__), WHITE) " " __VA_ARGS__)
#define log_fatalwloc(...) \
    log_printf(LOG_FATAL,  \
               ANSI_COLOR(__FILE__ ":" STR(__LINE__), WHITE) " " __VA_ARGS__)

#define log_pos_error(errf, ctx, tok, ...)  \
    do {                                    \
        log_printf(LOG_ERROR, __VA_ARGS__); \
        fprint_loc(errf, ctx, tok);         \
    } while (0)

#ifndef NDEBUG
    #define log_debug(...) log_printf(LOG_INFO, __VA_ARGS__)
#else
    #define log_debug(...)
#endif

enum log_type {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
};

void log_printf(int type, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#endif
