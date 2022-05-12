#include "top_level_parser.h"

struct init_declaration_list* parse_translation_unit(struct context* input) {
    struct init_declaration_list* ret_val = xmalloc(sizeof *ret_val);
    *ret_val = (struct init_declaration_list){
        .size = 0,
        .vars = NULL,
        .init_values = NULL,
    };
    while (!token_feof(input)) {
        // first read the new batch of declarations
        struct init_declaration_list* tmp = parse_declaration(input);
        // prepare ret_val for moving the values
        ret_val->size += tmp->size;
        // realloc the stuff inside ret_val
        size_t s = ret_val->size + tmp->size;
        ret_val->vars = xrealloc(ret_val->vars, s * sizeof *ret_val->vars);
        ret_val->init_values
            = xrealloc(ret_val->init_values, s * sizeof *ret_val->init_values);
        // then copy stuff over
        memcpy(&ret_val->vars[ret_val->size], tmp->vars,
               tmp->size * sizeof *tmp->vars);
        memcpy(&ret_val->init_values[ret_val->size], tmp->init_values,
               tmp->size * sizeof *tmp->init_values);
        // free the temporary thing
        free(tmp);
        ret_val->size = s;
    }
    return ret_val;
}
