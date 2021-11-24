#ifndef CONTEXT_H_
#define CONTEXT_H_

#include "new_tokenizer.h"

//struct context;
struct context {
    FILE* file;
    int stack_size;
    int* stack_head;
    struct token buffer[2];
    unsigned buffer_size;
    int token;
    unsigned long long row; // 1 indexed
    unsigned long long col; // 1 indexed
    int stack[];
};

struct context* create_ctx(FILE* f);
int token_getc(struct context* ctx);
void token_ungetc(int c, struct context* ctx);
int token_feof(struct context* ctx);

#endif
