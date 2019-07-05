#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "token.h"

int lex_tk_punct(struct lex_token *out, const char *name)
{
	assert(out != NULL);
	assert(name != NULL);
	int id = -1;

#define XX(a, b, c)             \
	if (!strcmp(name, c)) { \
		assert(id < 0); \
		id = b;         \
	}
	PUNCT_LIST(XX)
#undef XX

	assert(id >= 0);

	out->id = id;
	out->lexeme = name;

	return id;
}

int lex_tk_keyword(struct lex_token *out, const char *name)
{
	assert(name != NULL);
	assert(out != NULL);
	int id = -1;

#define XX(a, b)                 \
	if (!strcmp(name, #b)) { \
		id = a;          \
	}
	KEYWORD_LIST(XX)
#undef XX

	assert(id >= 0);

	out->id = id;
	out->lexeme = name;
	return out->id;
}

int lex_tk_iconst(struct lex_token *out, const char *str, size_t len)
{
	assert(str != NULL);
	assert(out != NULL);

	assert(len + 1 < MAX_TKLEN);
	char tmp[MAX_TKLEN] = { 0 };
	memcpy(tmp, str, len);

	char *end = tmp;

	long long num = strtoll(tmp, &end, 10);
	if (num == LONG_MIN || num == LONG_MAX) {
		if (errno == ERANGE) {
			fputs("number constant too big!", stderr);
			return LEX_TKINVALID;
		}
	}

	assert(end != str);

	out->id = LEX_TKICONST;
	out->iconst = num;
	out->lexeme = "<number>";

	return out->id;
}

int lex_id(const char *str)
{
	/* check if punctuator */

#define XX(a, b, c)            \
	if (!strcmp(str, c)) { \
		return a;      \
	}
	PUNCT_LIST(XX)
#undef XX

	/* check if keyword */
#define XX(a, b)                \
	if (!strcmp(str, #b)) { \
		return a;       \
	}
	KEYWORD_LIST(XX)
#undef XX

	/* invalid otherwise */
	assert(!"invalid str in lex_tk_id() call");
	return LEX_TKINVALID;
}

int lex_tk_identifier(struct lex_token *out, const char *str)
{
	assert(isalpha(*str) || *str == '_');
	size_t len = 0;

	while (lex_isidchar(str[len]))
		len++;

	char *tmp = malloc(len + 1);
	if (!tmp) {
		perror("malloc()");
		return LEX_TKINVALID;
	}

	memcpy(tmp, str, len);
	tmp[len] = '\0';

	out->id = LEX_TKIDENTIFIER;
	out->lexeme = tmp;
	return out->id;
}

bool lex_iskeyword(const char *str)
{
#define XX(a, b)                \
	if (!strcmp(str, #b)) { \
		return true;    \
	}
	KEYWORD_LIST(XX)
#undef XX
	return false;
}
