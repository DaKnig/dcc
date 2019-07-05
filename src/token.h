#ifndef TOKEN_H_
#define TOKEN_H_

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <errno.h>

#include <ctype.h>
#include <stdbool.h>

#define MAX_TKLEN 64
#define LEX_TKINVALID (-1)
#define LEX_TKEOI 0
#define LEX_TOKEN_INIT(x, y) \
	((struct lex_token){ .id = LEX_TKINVALID, .line = (y), .col = (x) })

#define PUNCT_LIST(XX)                  \
	XX(LEX_TKRTBRACE, '}', "}")     \
	XX(LEX_TKLTBRACE, '{', "{")     \
	XX(LEX_TKRTPAREN, ')', ")")     \
	XX(LEX_TKLTPAREN, '(', "(")     \
	XX(LEX_TKRTBRACK, ']', "]")     \
	XX(LEX_TKLTBRACK, '[', "[")     \
	XX(LEX_TKEQUAL, '=', "=")       \
	XX(LEX_TKGT, '>', ">")          \
	XX(LEX_TKLT, '<', "<")          \
	XX(LEX_TKDOT, '.', ".")         \
	XX(LEX_TKSEMICOLON, ';', ";")   \
	XX(LEX_TKCOLON, ':', ":")       \
	XX(LEX_TKQMARK, '?', "?")       \
	XX(LEX_TKCOMMA, ',', ",")       \
	XX(LEX_TKNEG, '~', "~")         \
	XX(LEX_TKNOT, '!', "!")         \
	XX(LEX_TKMOD, '%', "%")         \
	XX(LEX_TKXOR, '^', "^")         \
	XX(LEX_TKOR, '|', "|")          \
	XX(LEX_TKAND, '&', "&")         \
	XX(LEX_TKSLASH, '/', "/")       \
	XX(LEX_TKSTAR, '*', "*")        \
	XX(LEX_TKMINUS, '-', "-")       \
	XX(LEX_TKPLUS, '+', "+")        \
	XX(LEX_TKPLUSPLUS, 128, "++")   \
	XX(LEX_TKPLUSEQ, 129, "+=")     \
	XX(LEX_TKMINUSMINUS, 130, "--") \
	XX(LEX_TKMINUSEQ, 131, "-=")    \
	XX(LEX_TKARROW, 132, "->")      \
	XX(LEX_TKSTAREQ, 133, "*=")     \
	XX(LEX_TKSLASHEQ, 134, "/=")    \
	XX(LEX_TKANDAND, 135, "&&")     \
	XX(LEX_TKANDEQ, 136, "&=")      \
	XX(LEX_TKOROR, 137, "||")       \
	XX(LEX_TKOREQ, 138, "|=")       \
	XX(LEX_TKXOREQ, 139, "^=")      \
	XX(LEX_TKMODEQ, 140, "%=")      \
	XX(LEX_TKNOTEQ, 141, "!=")      \
	XX(LEX_ELLIPSIS, 142, "...")    \
	XX(LEX_TKLTLTEQ, 143, "<<=")    \
	XX(LEX_TKLTLT, 144, "<<")       \
	XX(LEX_TKLTEQ, 145, "<=")       \
	XX(LEX_TKGTGTEQ, 146, ">>=")    \
	XX(LEX_TKGTGT, 147, ">>")       \
	XX(LEX_TKGTEQ, 148, ">=")       \
	XX(LEX_TKEQEQ, 149, "==")

#define KEYWORD_LIST(XX)                         \
	XX(LEX_TKAUTO, auto)                     \
	XX(LEX_TKBREAK, break)                   \
	XX(LEX_TKCASE, case)                     \
	XX(LEX_TKCHAR, char)                     \
	XX(LEX_TKCONST, const)                   \
	XX(LEX_TKCONTINUE, continue)             \
	XX(LEX_TKDEFAULT, default)               \
	XX(LEX_TKDO, do)                         \
	XX(LEX_TKDOUBLE, double)                 \
	XX(LEX_TKELSE, else)                     \
	XX(LEX_TKENUM, enum)                     \
	XX(LEX_TKEXTERN, extern)                 \
	XX(LEX_TKFLOAT, float)                   \
	XX(LEX_TKFOR, for)                       \
	XX(LEX_TKGOTO, goto)                     \
	XX(LEX_TKIF, if)                         \
	XX(LEX_TKINLINE, inline)                 \
	XX(LEX_TKINT, int)                       \
	XX(LEX_TKLONG, long)                     \
	XX(LEX_TKREGISTER, register)             \
	XX(LEX_TKRESTRICT, restrict)             \
	XX(LEX_TKRETURN, return )                \
	XX(LEX_TKSHORT, short)                   \
	XX(LEX_TKSIGNED, signed)                 \
	XX(LEX_TKSIZEOF, sizeof)                 \
	XX(LEX_TKSTATIC, static)                 \
	XX(LEX_TKSTRUCT, struct)                 \
	XX(LEX_TKSWITCH, switch)                 \
	XX(LEX_TKTYPEDEF, typedef)               \
	XX(LEX_TKUNION, union)                   \
	XX(LEX_TKUNSIGNED, unsigned)             \
	XX(LEX_TKVOID, void)                     \
	XX(LEX_TKVOLATILE, volatile)             \
	XX(LEX_TKWHILE, while)                   \
	XX(LEX_TK_ALIGNAS, _Alignas)             \
	XX(LEX_TK_ALIGNOF, _Alignof)             \
	XX(LEX_TK_ATOMIC, _Atomic)               \
	XX(LEX_TK_BOOL, _Bool)                   \
	XX(LEX_TK_COMPLEX, _Complex)             \
	XX(LEX_TK_GENERIC, _Generic)             \
	XX(LEX_TK_IMAGINARY, _Imaginary)         \
	XX(LEX_TK_NORETURN, _Noreturn)           \
	XX(LEX_TK_STATIC_ASSERT, _Static_assert) \
	XX(LEX_TK_THREAD_LOCAL, _Thread_local)

enum lex_tk_id {
/* punctuation */
#define XX(a, b, c) a = b,
	PUNCT_LIST(XX)
#undef XX
	/* constants */
	LEX_TKICONST,
	LEX_TKILCONST,
	LEX_TKILLCONST,
	LEX_TKUCONST,
	LEX_TKULCONST,
	LEX_TKULLCONST,
	LEX_TKFLTCONST,
	LEX_TKDBLCONST,
	LEX_TKCHRCONST,
/* keywords */
#define XX(a, b) a,
	KEYWORD_LIST(XX)
#undef XX
	/* identifier */
	LEX_TKIDENTIFIER,
};

struct lex_token {
	int id;

	int line, col;

	const char *lexeme;

	union {
		intmax_t iconst;
		uintmax_t uconst;
		float fltconst;
		double dblconst;
		long double ldblconst;
		int chrconst;
	};
};

int lex_tk_punct(struct lex_token *out, const char *name);
int lex_tk_keyword(struct lex_token *out, const char *name);
int lex_tk_iconst(struct lex_token *out, const char *str, size_t len);
int lex_tk_identifier(struct lex_token *out, const char *str);

int lex_id(const char *str);
bool lex_iskeyword(const char *str);

static inline int lex_isidchar(int c)
{
	return isalnum(c) || (c == '_');
}

static inline const char *lex_tk_str(struct lex_token *tk)
{
	return tk->lexeme;
}

#endif
