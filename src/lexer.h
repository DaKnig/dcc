#ifndef LEXER_H_
#define LEXER_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include "token.h"

#define MAX_UNESCAPEDLEN 4

#define LEX_CTX_INIT()                             \
	((struct lex_context){                     \
		.next = NULL,                      \
		.end = NULL,                       \
		.lookahead = LEX_TOKEN_INIT(0, 0), \
		.token = LEX_TOKEN_INIT(0, 0),     \
		.stream = NULL,                    \
		.line = 0,                         \
		.col = 0,                          \
	})

struct lex_context {
	char buf[1 << 16];
	const char *next, *end;

	struct lex_token token;
	struct lex_token lookahead;

	const char *filename;

	FILE *stream;
	int line, col;
};

void lex_inits(struct lex_context *ctx, const char *str);
void lex_initf(struct lex_context *ctx, FILE *file);
void lex_setinputname(struct lex_context *ctx, const char *name);
int lex_nextwdest(struct lex_context *ctx, struct lex_token *out);

static inline int lex_peek(struct lex_context *ctx)
{
	return lex_nextwdest(ctx, &ctx->lookahead);
}

static inline int lex_next(struct lex_context *ctx)
{
	return lex_nextwdest(ctx, &ctx->token);
}

static inline bool lex_ioerror(struct lex_context *ctx)
{
	if (!ctx->stream)
		return false;
	return ferror(ctx->stream) != 0;
}

static inline struct lex_token *lex_curtk(struct lex_context *ctx)
{
	return &ctx->token;
}

#endif
