#include "context.h"

#include "log.h"
#include "tokenizer.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void token_push(struct context* ctx, int c) {
    //use at your own risk
    if (ctx->stack_head - ctx->stack == ctx->stack_size)
        fprintf(stderr, "context stack size is reached");
    *ctx->stack_head = c;
    ctx->stack_head++;
}

int token_pop(struct context* ctx) {
    if (ctx->stack_head == ctx->stack) log_error("popping empty context");
    ctx->stack_head--;
    return *ctx->stack_head;
}

int token_getc(struct context* ctx) {
    int ret_val;
    if (ctx->stack_head == ctx->stack) ret_val = fgetc(ctx->file);
    else
        ret_val = token_pop(ctx);
    ctx->col += !token_feof(ctx);
    if (ret_val == '\n') {
        ctx->col = 0;
        ctx->row++;
    }

#ifdef DEBUG
    fprintf(stderr, "c: '%c' ; col: %zd\n", ret_val, ctx->col);

#endif

    return ret_val;
}

void token_ungetc(int c, struct context* ctx) {
    if (c != EOF && c != (char)EOF) {
        token_push(ctx, c);
        ctx->col--;
    }
}

int token_feof(struct context* ctx) {
    if (ctx->stack_head == ctx->stack)
        return feof(ctx->file) || peek(ctx)->t == t_EOF;
    else
        return 0;
}

struct context* create_ctx(FILE* f) {
    const int size_of_stack = 1; // ought to be enough. increase as needed.
    struct context* c
        = xmalloc(sizeof(struct context) + size_of_stack * sizeof(int));
    c->file = f;
    c->stack_size = 1;
    c->stack_head = c->stack;
    c->buffer_size = 1 << 15;
	c->buffer[0] = (struct token) {
		.str = xmalloc(c->buffer_size),
		.t = t_bad_token
	};
	c->buffer[1] = (struct token) {
		.str = xmalloc(c->buffer_size),
		.t = t_bad_token
	};
    c->token = 0;
    c->row = c->col = 0; // 1 indexed

    token_ungetc(' ', c); // initialize unused stack
    next(c); // put one valid token into it
    return c;
}

void free_ctx(struct context *ctx) {
    if (ctx->file)
		fclose(ctx->file);
	free(ctx->buffer[0].str);
	free(ctx->buffer[1].str);
	free(ctx);
}

void fprint_loc(FILE* out, struct context* ctx, const struct token* t) {
    if (fseek(ctx->file, 0, SEEK_CUR) == -1) {
#ifdef DEBUG
        fprintf(stderr, "file unseekable\n");
#endif
        return;
    }
    // first, rewind to beginning of file/start of line
    fsetpos(ctx->file, &t->pos);
    size_t col = t->col + 1;
    do {
        long seek_by = col > (1 << 30) ? (1 << 30) : col;
        col -= seek_by;
        fseek(ctx->file, -seek_by, SEEK_CUR);
    } while (col > 0);
    // now, print the line and simultaneously record the spaces
    size_t n;
    char* str = xmalloc(256);
    n = fprintf(out, "%7zi | ", t->row + 1); // 1 indexed
    memset(str, ' ', n); // place that many spaces on str

    int c;
    for (c = getc(ctx->file), col = t->col;
         c != EOF && col; // only care about recording spaces up to col
         c = getc(ctx->file), col--) {

        if (n % 128) { str = realloc(str, n + 128); }
        fputc(c, out); // print char
        str[n++] = " \t"[c == '\t']; // save the right whitespace
    }
    while (c != EOF && c != '\n') {
        fputc(c, out);
        c = getc(ctx->file);
    }
    fputc('\n', out);
    str[n] = '\0';
    fprintf(out, "%s^\n", str);
    free(str);
}
