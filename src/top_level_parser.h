#ifndef TOP_LEVEL_PARSER
#define TOP_LEVEL_PARSER

#include "decl_parser.h"
#include "statement_parser.h"

struct c_func {
    char* name;
    struct decl_type declaration;
    struct block body;
};

struct external_declaration {
    enum { e_d_declaration, e_d_function } t;
    union {
		struct init_declaration_list declaration;
		struct c_func function;
	};
};

struct init_declaration_list parse_translation_unit(struct context* input);

#endif // TOP_LEVEL_PARSER
