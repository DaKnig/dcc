#ifndef PRATT_H
#define PRATT_H

enum ast_type   {term, unary_op, bin_op, ternary_op,
                comma, func_call, unprocessed};

struct token* token_stream_head;
struct token* token_stream;
unsigned token_stream_len;


struct ast{
    enum ast_type type;
    union{
        struct{//term
            struct token* token;
        };
        struct{//op - unary, binary, ternary
            struct token* op;
            struct ast* term[3];
        };
        struct{//func call,comma///fix is required!!!
            struct token* func_name;
            struct ast* *argv;
            int argc;
            //pointer to a block
        };
    };
};

struct ast* expr(int bp);
void print_ast(struct ast* root, int indent);
void free_ast(struct ast* root);

#endif
