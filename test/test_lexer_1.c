#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <errno.h>

#include "token.h"
#include "lexer.h"

void test_puncts(void)
{
	const char allpuncts[] =
#define XX(a, b, c) c " "
		PUNCT_LIST(XX)
#undef XX
		;

	struct lex_context ctx;
	lex_inits(&ctx, allpuncts);

#define XX(a, b, c)                                                     \
	do {                                                            \
		const int id = lex_getnext(&ctx);                       \
		printf("id: %3d lexeme: '%s'\n", id, ctx.token.lexeme); \
                                                                        \
		fflush(stdout);                                         \
		assert(id == a);                                        \
                                                                        \
		const char *lexeme = ctx.token.lexeme;                  \
		assert(strcmp(lexeme, c) == 0);                         \
	} while (0);
	PUNCT_LIST(XX)
#undef XX
}

void test_keywords(void)
{
	const char allkws[] =
#define XX(a, b) #b " "
		KEYWORD_LIST(XX)
#undef XX
		;

	struct lex_context ctx;
	lex_inits(&ctx, allkws);

#define XX(a, b)                                                        \
	do {                                                            \
		const int id = lex_getnext(&ctx);                       \
		printf("id: %3d lexeme: '%s'\n", id, ctx.token.lexeme); \
                                                                        \
		fflush(stdout);                                         \
		assert(id == a);                                        \
                                                                        \
		const char *lexeme = ctx.token.lexeme;                  \
		assert(strcmp(lexeme, #b) == 0);                        \
	} while (0);
	KEYWORD_LIST(XX)
#undef XX
}

int main(void)
{
	test_puncts();
	test_keywords();
	return 0;
}
