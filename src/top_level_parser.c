#include "top_level_parser.h"

#include "pratt.h"
#include "decl_parser.h"
#include "log.h"

struct init_declaration_list parse_translation_unit(struct context* input) {
    // translation-unit :
    //           external-declaration
    //           translation-unit external-declaration

    // external-declaration :=
    //           declaration
    //           function-definition
    struct init_declaration_list ret_val = {
        .size = 0,
        .vars = NULL,
        .init_values = NULL,
    };
    while (!token_feof(input)) {
        // first read the new batch of declarations
        struct init_declaration_list tmp = parse_declaration(input);
        // realloc the stuff inside ret_val
        size_t s = ret_val.size + tmp.size;
        ret_val.vars = xrealloc(ret_val.vars, s * sizeof *ret_val.vars);
        ret_val.init_values
            = xrealloc(ret_val.init_values, s * sizeof *ret_val.init_values);
        // then copy stuff over
        memcpy(&ret_val.vars[ret_val.size], tmp.vars,
               tmp.size * sizeof *tmp.vars);
        memcpy(&ret_val.init_values[ret_val.size], tmp.init_values,
               tmp.size * sizeof *tmp.init_values);
        ret_val.size = s;

        tmp.size = 0; // content was moved out. delete the container.
        free_translation_unit(&tmp);
    }
    return ret_val;
}

static inline struct external_declaration*
    parse_external_declaration(struct context* input) {
    // external-declaration :=
    //           function-definition
    //           declaration

    // function-definition :=
    //           declaration-specifiers declarator compound-statement

    // declaration :=
    //           declaration-specifiers [init-declaration-list] ";"

    // so, we need to parse until after the first declarator to determine if
    // the next token is "{" (function-definition) or something else
    // (declaration)

    // 1. read in the declarator-specifiers
    struct decl_specifiers specs = get_decl_specifiers(input);
    // 2. is it a `;`? if so, we have got nothing
    const struct token* t = peek(input);
    if (t->str[0] == ';') {
        // empty declaration
        log_pos_warning(stderr, input, t,
                        "useless type name in empty declaration!");
        next(input); // eat up that ';'
        return NULL;
    }
    // now there must be at least one declarator
    struct decl_type declarator = get_declarator(input, specs);

    t = peek(input);
    switch (t->str[0]) {
    case '{': // function definition
        // first check that it is a function declarator
        if (declarator.t != d_function) {
            log_pos_error(stderr, input, t,
                          "expected function declarator before "
                          "function-body");
        }
        // next take in the function body
        struct statement* body_statement = parse_statement(input);
        if (body_statement->t != s_compound) {
            log_pos_error(stderr, input, t,
                          "internal error: expected compound statement as"
                          "function-body");
        }

        struct block body_block = body_statement->block;
        free(body_statement);

        struct external_declaration* ret_val = xmalloc(sizeof *ret_val);

        // get the name of the variable
        struct decl_type* d;
        for (d = &declarator; d->t != d_base;) {
            if (d->t == d_ptr || d->t == d_array) {
                d = d->declarator;
            } else {
                log_error("unknown decl type with ID=%d\n", (int)d->t);
            }
        }

        *ret_val = (struct external_declaration){
            .t = e_d_function,
            .function
            = (struct c_func){.body = body_block, .declaration = declarator}};
        ret_val->function.declaration.ret_type->specifiers = specs;
        return ret_val;
    case '=':
    case ',':
    case ';': // declaration
    default:
        log_pos_error(stderr, input, t, "expected '=', ',', ';' or '{' here");
    }
    /* struct external_declaration* ret_val = NULL; */
    return NULL;
}

void free_translation_unit(struct init_declaration_list* tu) {
    for (size_t i = 0; i < tu->size; i++) {
        free_c_var(&tu->vars[i]);
        if (tu->init_values[i]) free_expr_ast(tu->init_values[i]);
    }

    free(tu->init_values);
    free(tu->vars);
}
