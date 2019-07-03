#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

#include "token.h"
#include "lexer.h"

static size_t refill(struct lex_context *ctx)
{
	const int have = ctx->end - ctx->next;
	assert(have >= 0);

	if (!ctx->stream)
		return 0;

	if (ctx->next)
		memmove(ctx->buf, ctx->next, have);

	ctx->next = ctx->buf;
	ctx->end = ctx->buf + have;

	const size_t n = fread(ctx->buf + have, 1, sizeof(ctx->buf) - have - 1,
			       ctx->stream);

	ctx->end += n;
	ctx->buf[have + n] = '\n';
	return n;
}

void lex_inits(struct lex_context *ctx, const char *str)
{
	assert(ctx != NULL);
	*ctx = LEX_CTX_INIT();

	ctx->next = str;
	ctx->end = str + strlen(str);
}

void lex_initf(struct lex_context *ctx, FILE *file)
{
	assert(ctx != NULL);
	*ctx = LEX_CTX_INIT();

	ctx->stream = file;
	refill(ctx);
}

static int isidchar(int c)
{
	return isalnum(c) || (c == '_');
}

#define HANDLE_KEYWORD(str)                                          \
	do {                                                         \
		if (!strncmp(ctx->next, str, strlen(str))) {         \
			if (!isidchar(ctx->next[strlen(str)])) {     \
				lex_tk_keyword(out, str, line, col); \
				goto done;                           \
			}                                            \
		}                                                    \
	} while (0)

#define HANDLE_PUNCTUATOR(str)                               \
	do {                                                 \
		if (!strncmp(ctx->next, str, strlen(str))) { \
			lex_tk_punct(out, str, line, col);   \
			goto done;                           \
		}                                            \
	} while (0)

int lex_getnext(struct lex_context *ctx)
{
	assert(ctx != NULL);
	struct lex_token *const out = &ctx->token;

	do {
		if (ctx->next >= ctx->end) {
			if (!refill(ctx)) {
				if (lex_ioerror(ctx))
					goto io_error;
				out->id = LEX_TKEOI;
				out->lexeme = "<EOI>";
				out->line = ctx->line;
				out->col = ctx->col;
				goto done;
			}
		}
		if (*ctx->next == '\n') {
			ctx->line++;
			ctx->col = 0;
		} else
			ctx->col++;
	} while (isspace(*ctx->next++));

	ctx->next = ctx->next - 1;
	ctx->col -= 1;

	if (ctx->end - ctx->next < MAX_TKLEN) {
		if (!refill(ctx))
			if (lex_ioerror(ctx))
				goto io_error;
	}

	const int line = ctx->line;
	const int col = ctx->col;

	switch (*ctx->next) {
	case '+':
		HANDLE_PUNCTUATOR("++");
		HANDLE_PUNCTUATOR("+=");
		HANDLE_PUNCTUATOR("+");
		break;
	case '-':
		HANDLE_PUNCTUATOR("--");
		HANDLE_PUNCTUATOR("-=");
		HANDLE_PUNCTUATOR("->");
		HANDLE_PUNCTUATOR("-");
		break;
	case '*':
		HANDLE_PUNCTUATOR("*=");
		HANDLE_PUNCTUATOR("*");
		break;
	case '/':
		HANDLE_PUNCTUATOR("/=");
		HANDLE_PUNCTUATOR("/");
		break;
	case '&':
		HANDLE_PUNCTUATOR("&&");
		HANDLE_PUNCTUATOR("&=");
		HANDLE_PUNCTUATOR("&");
		break;
	case '|':
		HANDLE_PUNCTUATOR("||");
		HANDLE_PUNCTUATOR("|=");
		HANDLE_PUNCTUATOR("|");
		break;
	case '^':
		HANDLE_PUNCTUATOR("^=");
		HANDLE_PUNCTUATOR("^");
		break;
	case '~':
		HANDLE_PUNCTUATOR("~");
		break;
	case '%':
		HANDLE_PUNCTUATOR("%=");
		HANDLE_PUNCTUATOR("%");
		break;
	case '!':
		HANDLE_PUNCTUATOR("!=");
		HANDLE_PUNCTUATOR("!");
		break;
	case ',':
		HANDLE_PUNCTUATOR(",");
		break;
	case '?':
		HANDLE_PUNCTUATOR("?");
		break;
	case ':':
		HANDLE_PUNCTUATOR(":");
		break;
	case ';':
		HANDLE_PUNCTUATOR(";");
		break;
	case '.':
		HANDLE_PUNCTUATOR("...");
		HANDLE_PUNCTUATOR(".");
		break;
	case '<':
		HANDLE_PUNCTUATOR("<<=");
		HANDLE_PUNCTUATOR("<=");
		HANDLE_PUNCTUATOR("<<");
		HANDLE_PUNCTUATOR("<");
		break;
	case '>':
		HANDLE_PUNCTUATOR(">>=");
		HANDLE_PUNCTUATOR(">=");
		HANDLE_PUNCTUATOR(">>");
		HANDLE_PUNCTUATOR(">");
		break;
	case '=':
		HANDLE_PUNCTUATOR("==");
		HANDLE_PUNCTUATOR("=");
		break;
	case '[':
		HANDLE_PUNCTUATOR("[");
		break;
	case ']':
		HANDLE_PUNCTUATOR("]");
		break;
	case '(':
		HANDLE_PUNCTUATOR("(");
		break;
	case ')':
		HANDLE_PUNCTUATOR(")");
		break;
	case '{':
		HANDLE_PUNCTUATOR("{");
		break;
	case '}':
		HANDLE_PUNCTUATOR("}");
		break;
	case '0':
		if (ctx->next[1] == 'x')
			assert(!"hex constants not implemented");
		assert(!"octal constants not implemented");
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': {
		const char *begin = ctx->next;
		const char *end = ctx->next;
		while (isdigit(*end))
			end++;

		if (*end == '.') {
			assert(!"fp constants not implemented");
		}

		if (*end == 'E' || *end == 'e')
			assert(!"fp constants not implemented");

		if (*end == 'u' || *end == 'U') {
			assert(!"unsigned constants not implemented");
		}

		if (*end == 'l' || *end == 'L') {
			assert(!"long constants not implemented");
		}

		ctx->next = end;
		return lex_tk_iconst(out, begin, end - begin, line, col);
	} break;
	case '_':
		HANDLE_KEYWORD("_Static_assert");
		HANDLE_KEYWORD("_Thread_local");
		HANDLE_KEYWORD("_Imaginary");
		HANDLE_KEYWORD("_Noreturn");
		HANDLE_KEYWORD("_Complex");
		HANDLE_KEYWORD("_Generic");
		HANDLE_KEYWORD("_Alignas");
		HANDLE_KEYWORD("_Alignof");
		HANDLE_KEYWORD("_Atomic");
		HANDLE_KEYWORD("_Bool");
		goto identifier;
	case 'a':
		HANDLE_KEYWORD("auto");
		goto identifier;
	case 'b':
		HANDLE_KEYWORD("break");
		goto identifier;
	case 'c':
		HANDLE_KEYWORD("continue");
		HANDLE_KEYWORD("const");
		HANDLE_KEYWORD("case");
		HANDLE_KEYWORD("char");
		goto identifier;
	case 'd':
		HANDLE_KEYWORD("default");
		HANDLE_KEYWORD("double");
		HANDLE_KEYWORD("do");
		goto identifier;
	case 'e':
		HANDLE_KEYWORD("extern");
		HANDLE_KEYWORD("else");
		HANDLE_KEYWORD("enum");
		goto identifier;
	case 'f':
		HANDLE_KEYWORD("float");
		HANDLE_KEYWORD("for");
		goto identifier;
	case 'g':
		HANDLE_KEYWORD("goto");
		goto identifier;
	case 'h':
	case 'i':
		HANDLE_KEYWORD("inline");
		HANDLE_KEYWORD("int");
		HANDLE_KEYWORD("if");
		goto identifier;
	case 'j':
	case 'k':
	case 'l':
		HANDLE_KEYWORD("long");
		goto identifier;
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
		HANDLE_KEYWORD("register");
		HANDLE_KEYWORD("restrict");
		HANDLE_KEYWORD("return");
		goto identifier;
	case 's':
		HANDLE_KEYWORD("signed");
		HANDLE_KEYWORD("sizeof");
		HANDLE_KEYWORD("static");
		HANDLE_KEYWORD("struct");
		HANDLE_KEYWORD("switch");
		HANDLE_KEYWORD("short");
		goto identifier;
	case 't':
		HANDLE_KEYWORD("typedef");
		goto identifier;
	case 'u':
		HANDLE_KEYWORD("unsigned");
		HANDLE_KEYWORD("union");
		goto identifier;
	case 'v':
		HANDLE_KEYWORD("volatile");
		HANDLE_KEYWORD("void");
		goto identifier;
	case 'w':
		HANDLE_KEYWORD("while");
		goto identifier;
	case 'x':
	case 'y':
	case 'z':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
	identifier : {
		const char *begin = ctx->next;
		const char *end = begin + 1;

		while (isidchar(*end))
			end++;

		ctx->next = end;
		lex_tk_identifier(out, begin, end - begin, line, col);
	} break;
	case '"':
		assert(!"string literals not implemented");
		break;
	default:
		fprintf(stderr, "invalid char '%c'!\n", *ctx->next);
		goto error;
	}

done:
	assert(out->lexeme != NULL);

	ctx->next += strlen(out->lexeme);
	ctx->col += strlen(out->lexeme);

eoi:
	return out->id;
error:
	return LEX_TKINVALID;
io_error:
	fputs("i/o error occurred...\n", stderr);
	return LEX_TKINVALID;
}
