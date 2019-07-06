#ifndef TEST_H_
#define TEST_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

static inline void assert_die(const char *file, unsigned line, const char *func,
			      const char *expr)
{
	fflush(NULL);
	fprintf(stderr, "ASSERT FAILED %s:%u in %s(): %s\n", file, line, func,
		expr);
	fflush(stderr);

	abort();
}

#define TEST_ASSERT(cond)                                                    \
	do {                                                                 \
		if (cond) {                                                  \
		} else {                                                     \
			assert_die(__FILE__, __LINE__, __FUNCTION__, #cond); \
		}                                                            \
	} while (0)

#endif
