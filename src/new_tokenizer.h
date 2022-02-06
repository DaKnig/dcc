#ifndef NEW_TOKENIZER
#define NEW_TOKENIZER

#include <stdio.h>

struct context;

enum token_type {
    t_keyword,
    t_identifier,
    t_string,
    t_char,
    t_float,
    t_unknown,
    t_punctuator,
    t_bad_token,
    t_EOF,
    t_int
};

struct token {
    char* str;
    enum token_type t;
    union {
        unsigned long long number;
        double floating_number;
    };
    union {
        char string_prefix[4];
        char number_suffix[4];
    };
    size_t row;
    size_t col;
    fpos_t pos;
};

#include "context.h"

struct token* peek(struct context* input);
struct token* next(struct context* input);
void expect_token(const char* expected, struct context* input);

void print_token(struct token* token);

#endif
