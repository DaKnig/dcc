#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <errno.h>

#include "test.h"
#include "log.h"
#include "token.h"
#include "lexer.h"

int main(void)
{
	log_print("Testing peek() ...");

	struct lex_context lctx = LEX_CTX_INIT();
	lex_inits(&lctx, "\"test\"[a + b / c]");

	TEST_ASSERT(lex_peek(&lctx) == LEX_TKSTRING);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, "test"));

	TEST_ASSERT(lex_peek(&lctx) == '[');
	TEST_ASSERT(lex_next(&lctx) == '[');

	TEST_ASSERT(lex_peek(&lctx) == LEX_TKIDENTIFIER);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKIDENTIFIER);
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, "a"));

	TEST_ASSERT(lex_peek(&lctx) == '+');
	TEST_ASSERT(lex_next(&lctx) == '+');

	TEST_ASSERT(lex_peek(&lctx) == LEX_TKIDENTIFIER);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKIDENTIFIER);
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, "b"));

	TEST_ASSERT(lex_peek(&lctx) == '/');
	TEST_ASSERT(lex_next(&lctx) == '/');

	TEST_ASSERT(lex_peek(&lctx) == LEX_TKIDENTIFIER);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKIDENTIFIER);
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, "c"));

	TEST_ASSERT(lex_peek(&lctx) == ']');
	TEST_ASSERT(lex_next(&lctx) == ']');

	return 0;
}
