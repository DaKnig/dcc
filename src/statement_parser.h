#ifndef STATEMENT_PARSER
#define STATEMENT_PARSER

#include "decl_parser.h"

struct statement;
struct context;

struct block_element {
    enum { s_statement, s_declaration } t;
    struct statement* s;
    struct init_declaration_list* d;
};

struct block {
    unsigned size; //how many block block_elements do we have?
    struct block_element* e;
};

struct statement {
    enum {
        s_case,
        s_label,
        s_default, //labeled-statement
        s_compound, //compound-statement
        s_expression, //expression-statement
        s_if,
        s_if_else,
        s_switch, //selection-statement
        s_for,
        s_while,
        s_do_while, //iteration_statement
        s_goto,
        s_continue,
        s_break,
        s_return
    } t;
    //jump_statement
    union {
        struct block block;
        struct {
            union {
                struct statement* s;
                struct { //if-else
                    struct statement* if_true;
                    struct statement* if_false;
                };
            };
            union {
                struct expr_ast* e;
                char* label;
                struct expr_ast* expr[3];
            };
        };
    };
};

struct statement* parse_statement(struct context* input);
void print_statement(struct statement* s, int indent);

#endif
