#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "tokenizer.h"

//struct context;
struct context {
    FILE* file;
    int stack_size;
    int* stack_head;
    struct token buffer[2];
    unsigned buffer_size;
    int token;
    size_t row; // 0 indexed
    size_t col; // 0 indexed
    int stack[];
};

struct context* create_ctx(FILE* f);
void free_ctx(struct context *ctx);
int token_getc(struct context* ctx);
void token_ungetc(int c, struct context* ctx);
int token_feof(struct context* ctx);

void fprint_loc(FILE* out, struct context* ctx, const struct token* t);
// if context is not seekable, do nothing.
// otherwise, print the line from ctx starting from pos-col,
// with an indicator.
// col is 1-indexed- first char on the line has col=1.
// restores the state of ctx before continuing.

#endif
