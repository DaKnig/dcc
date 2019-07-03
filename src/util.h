#ifndef UTIL_H_
#define UTIL_H_

#if !defined(__WIN32__)
#define ANSI_RED "\033[31;1m"
#define ANSI_BLUE "\033[34;1m"
#define ANSI_BLACK "\033[30;1m"
#define ANSI_WHITE "\033[37;1m"
#define ANSI_YELLOW "\033[33;1m"
#define ANSI_RESET "\033[0m"
#else
#define ANSI_RED
#define ANSI_BLUE
#define ANSI_BLACK
#define ANSI_WHITE
#define ANSI_YELLOW
#define ANSI_RESET
#endif

#define ANSI_COLOR(s, c) ANSI_##c s ANSI_RESET

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef CEIL_DIV
#define CEIL_DIV(a, b) (((a) + (b)-1) / (b))
#endif

#ifndef ALIGN_TO
#define ALIGN_TO(a, b) ((b)*CEIL_DIV(a, b))
#endif

#ifndef COUNT_OF
#define COUNT_OF(v) ((sizeof(v) / sizeof(0 [v])))
#endif

static inline void *xmalloc(size_t sz)
{
	void *const ret = malloc(sz);
	if (!ret) {
		fprintf(stderr, "fatal: malloc(%zu): %s\n", sz,
			strerror(errno));
		abort();
	}
	return ret;
}

static inline void *xrealloc(void *block, size_t sz)
{
	void *ret = realloc(block, sz);
	if (!ret) {
		fprintf(stderr, "fatal: realloc(%p, %zu): %s\n", block, sz,
			strerror(errno));
		abort();
	}
	return ret;
}

#endif
