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

#include "util.h"
#include "log.h"
#include "token.h"
#include "lexer.h"

static inline size_t nhave(struct lex_context *ctx)
{
	assert(ctx->end >= ctx->next);
	return ctx->end - ctx->next;
}

static size_t refill(struct lex_context *ctx)
{
	size_t have = nhave(ctx);

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

static inline bool lex_isoctal(int c)
{
	return c >= '0' && c <= '7';
}

static inline bool lex_ishex(int c)
{
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static inline unsigned hexvalue(int c)
{
	if (isdigit(c))
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}

static int unescape(char *dest, const char **psrc)
{
	assert(psrc != NULL);
	int i = 0;

	const char *src = *psrc;
	assert(*src == '\\');

	switch (*++src) {
	case '\\':
		dest[i++] = '\\', ++src;
		break;
	case '\'':
		dest[i++] = '\'', ++src;
		break;
	case '"':
		dest[i++] = '\"', ++src;
		break;
	case 'a':
		dest[i++] = '\a', ++src;
		break;
	case 'b':
		dest[i++] = '\b', ++src;
		break;
	case 'f':
		dest[i++] = '\f', ++src;
		break;
	case 'n':
		dest[i++] = '\n', ++src;
		break;
	case 'r':
		dest[i++] = '\r', ++src;
		break;
	case 't':
		dest[i++] = '\t', ++src;
		break;
	case 'v':
		dest[i++] = '\v', ++src;
		break;
	case 'x': {
		unsigned value = 0;

		while (lex_ishex(*++src)) {
			unsigned tmp = (value << 4) + hexvalue(*src);
			if (tmp >= 256) {
				break;
			}
			value = tmp;
		}

		dest[i++] = value;
	} break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7': {
		unsigned value = 0;

		for (; lex_isoctal(*src); src++) {
			unsigned tmp = (value << 3) + (*src - '0');
			if (tmp >= 256) {
				break;
			}
			value = tmp;
		}

		dest[i++] = value;
	} break;
	default:
		log_error("bad escape sequence");
		return -1;
	}

	dest[i] = '\0';
	*psrc = src;

	return i;
}

static int handle_string(struct lex_context *ctx, struct lex_token *out)
{
	char *str = xmalloc(4);
	size_t i = 0, cap = 4;

	bool done = false;

	assert(*ctx->next == '"');

	while (*++ctx->next != '"') {
		switch (*ctx->next) {
		case '\0':
			if (!nhave(ctx)) {
				if (!refill(ctx)) {
					if (lex_ioerror(ctx)) {
						goto io_error;
					}
					goto missing_delim;
				}
				continue;
			}
			break;
		case '\n':
			goto missing_delim;
		case '\\':
			if (nhave(ctx) < 5) {
				refill(ctx);
				if (lex_ioerror(ctx)) {
					goto io_error;
				}
			}

			if (i + MAX_UNESCAPEDLEN + 1 >= cap) {
				cap = MAX(2 * cap,
					  ALIGN_TO(i + MAX_UNESCAPEDLEN + 1,
						   8));
				str = xrealloc(str, cap);
			}

			const int len = unescape(str + i, &ctx->next);
			if (len < 0) {
				goto error;
			}

			i += len;
			ctx->next--;
			continue;
		}

		if (i + 2 >= cap) {
			cap = MAX(2 * cap, ALIGN_TO(i + 2, 8));
			str = xrealloc(str, cap);
		}

		str[i++] = *ctx->next;
	}

	ctx->next++;

	assert(i + 1 <= cap);

	str[i] = '\0';
	str = xrealloc(str, i + 1);

done:
	return lex_tk_string(out, str);
io_error:
	log_error("i/o error ocurred:");
	goto error;
missing_delim:
	log_error("expected '\"' delimiter");
error:
	free(str);
	return -1;
}

#define HANDLE_KEYWORD(str)                                          \
	do {                                                         \
		if (!strncmp(ctx->next, str, strlen(str))) {         \
			if (!lex_isidchar(ctx->next[strlen(str)])) { \
				lex_tk_keyword(out, str);            \
				goto done;                           \
			}                                            \
		}                                                    \
	} while (0)

#define HANDLE_PUNCTUATOR(str)                               \
	do {                                                 \
		if (!strncmp(ctx->next, str, strlen(str))) { \
			lex_tk_punct(out, str);              \
			goto done;                           \
		}                                            \
	} while (0)

int lex_nextwdest(struct lex_context *ctx, struct lex_token *out)
{
	assert(ctx != NULL);

	if (ctx->lookahead.id != LEX_TKINVALID) {
		*out = ctx->lookahead;
		ctx->lookahead.id = LEX_TKINVALID;
		return out->id;
	}

	do {
		if (!nhave(ctx)) {
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

	if (nhave(ctx) < MAX_TKLEN) {
		if (!refill(ctx))
			if (lex_ioerror(ctx))
				goto io_error;
	}

	*out = LEX_TOKEN_INIT(ctx->line, ctx->col);

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
		assert(!"not implemented");

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
		lex_tk_iconst(out, begin, end - begin);
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
	identifier:
		lex_tk_identifier(out, ctx->next);
		break;
	case '"':
		return handle_string(ctx, out);
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

