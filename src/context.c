#include <stdlib.h>
#include <assert.h>

#include "log.h"
#include "context.h"
#include "new_tokenizer.h"

void token_push(struct context* ctx, int c){
    //use at your own risk
    if (ctx->stack_head-ctx->stack == ctx->stack_size)
        fprintf(stderr,"context stack size is reached");
    *ctx->stack_head=c;
    ctx->stack_head++;
}

int token_pop(struct context* ctx) {
    if (ctx->stack_head==ctx->stack)
        log_error("popping empty context");
    ctx->stack_head--;
    return *ctx->stack_head;
}

int token_getc(struct context* ctx) {
    int ret_val;
    if (ctx->stack_head==ctx->stack)
        ret_val = fgetc(ctx->file);
    else
        ret_val = token_pop(ctx);
    ctx->col++;
    if (ret_val=='\n') {
	ctx->col=1;
	ctx->row++;
    }
    return ret_val;
}

void token_ungetc(int c, struct context* ctx) {
    if (c!=EOF &&c!=(char)EOF)
        token_push(ctx,c);
    ctx->col--;
}

int token_feof(struct context* ctx) {
    if (ctx->stack_head==ctx->stack)
        return feof(ctx->file);
    else
        return 0;
}

struct context* create_ctx(FILE* f){
    struct context* c = malloc(sizeof(struct context)+5*sizeof(int));
    c->file=f;
    c->stack_size=5;
    c->stack_head=c->stack;
    c->buffer_size=1<<15;
    c->buffer[0].str=malloc(c->buffer_size);
    c->buffer[1].str=malloc(c->buffer_size);
    assert("malloc"&&c->buffer[0].str&&c->buffer[1].str);
    c->token=0;
    c->row = c->col = 1; // 1 indexed
    next(c);    // put one valid token into it
    return c;
}
