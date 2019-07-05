#ifndef LEXER_H_
#define LEXER_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include "token.h"

#define LEX_CTX_INIT()                         \
	((struct lex_context){                 \
		.next = NULL,                  \
		.end = NULL,                   \
		.token = LEX_TOKEN_INIT(0, 0), \
		.stream = NULL,                \
		.line = 0,                     \
		.col = 0,                      \
	})

struct lex_context {
	char buf[1 << 16];
	const char *next, *end;

	struct lex_token token;

	const char *filename;

	FILE *stream;
	int line, col;
};

void lex_inits(struct lex_context *ctx, const char *str);
void lex_initf(struct lex_context *ctx, FILE *file);
void lex_setinputname(struct lex_context *ctx, const char *name);
int lex_getnext(struct lex_context *ctx);

static inline bool lex_ioerror(struct lex_context *ctx)
{
	if (!ctx->stream)
		return false;
	return ferror(ctx->stream) != 0;
}

#endif
