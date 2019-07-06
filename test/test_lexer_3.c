#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <errno.h>

#include "test.h"
#include "log.h"
#include "token.h"
#include "lexer.h"

void testcase1(void)
{
	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, "\"\"");
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, ""));
}

void testcase2(void)
{
	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, "\"\\nfoobar\\v\"");
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, "\nfoobar\v"));
}

void testcase3(void)
{
	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, "          \"hello world\"");
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, "hello world"));
}

void testcase4(void)
{
	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, "\"\\n\\a\\t\\r\\v\\b\\\\\"");
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, "\n\a\t\r\v\b\\"));
}

void testcase5(void)
{
	static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz"
				       "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	static char longstr[(1 << 16) + 1];
	const size_t n = sizeof(longstr);

	assert(n > 2);

	longstr[0] = '"';

	for (size_t i = 1; i < n - 2; i++) {
		longstr[i] = alphabet[i % (sizeof(alphabet) - 1)];
	}

	longstr[n - 2] = '"';
	longstr[n - 1] = '\0';

	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, longstr);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);

	longstr[n - 2] = '\0';
	TEST_ASSERT(!strcmp(lex_curtk(&lctx)->lexeme, longstr + 1));
}

void testcase6(void)
{
	const char str[] = "\"\\xFF0\"";
	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, str);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);

	struct lex_token *tk = lex_curtk(&lctx);
	TEST_ASSERT(tk->lexeme[0] == '\xFF');
}

void testcase7(void)
{
	const char str[] = "\"\\xab\"";
	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, str);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);

	struct lex_token *tk = lex_curtk(&lctx);
	TEST_ASSERT(tk->lexeme[0] == '\xab');
}

void testcase8(void)
{
	const char str[] = "\"\\x0\"";
	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, str);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);

	struct lex_token *tk = lex_curtk(&lctx);
	TEST_ASSERT(tk->lexeme[0] == '\x0');
}

void testcase9(void)
{
	static char str[1 << 16];
	size_t n = 1;
	unsigned i;

	str[0] = '"';

	for (i = 0; i < 256; i++) {
		n += snprintf(str + n, sizeof(str) - n, "\\x%02x", i);
	}

	assert(n + 1 < sizeof(str));
	str[n] = '"';
	str[n + 1] = '\0';

	struct lex_context lctx = LEX_CTX_INIT();

	lex_inits(&lctx, str);
	TEST_ASSERT(lex_next(&lctx) == LEX_TKSTRING);

	struct lex_token *tk = lex_curtk(&lctx);

	for (i = 0; i < 256; i++) {
		TEST_ASSERT((unsigned char)tk->lexeme[i] == i);
	}
}

int main(void)
{
	log_print("Testing string literals...\n");

	testcase1();
	testcase2();
	testcase3();
	testcase4();
	testcase5();
	testcase6();
	testcase7();
	testcase8();
	testcase9();

	return 0;
}
