#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "util.h"
#include "lexer.h"

int main(void)
{
	struct lex_context ctx;
	lex_inits(&ctx, "  a aa a2a b bb \nbbb fowhi \tfo le doubls     ");

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "a") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "aa") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "a2a") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "b") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "bb") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "bbb") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "fowhi") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "fo") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "le") == 0);

	assert(lex_next(&ctx) == LEX_TKIDENTIFIER);
	assert(strcmp(ctx.token.lexeme, "doubls") == 0);

	return 0;
}
