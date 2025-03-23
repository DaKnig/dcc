#include "decl_parser.h"

#include "assert.h"
#include "log.h"
#include "pratt.h"
#include "util.h"

#include <stdbool.h>

struct decl_specifiers get_decl_specifiers(struct context* input) {
    // syntax described at docs/declaration-specifier.dot
    struct decl_specifiers ret_val;
    ret_val = (struct decl_specifiers){
        .storage_class = s_c_none,

        .is_const = false,
        .is_restrict = false,
        .is_volatile = false,
        .is_inline = false,

        .signedness = t_s_none,
        .length_modifier = t_s_natural,
        .core_type = t_s_typeless,
    };

    unsigned s_c_so_far = 0;
    for (const struct token* t = peek(input);
         t->t == t_keyword; //otherwise its out of declaration-specifiers
         next(input), t = peek(input)) {

#define XX(KEYWORD) /* typ-specifier - core type*/            \
    if (strcmp(t->str, #KEYWORD) == 0) {                      \
        if (ret_val.core_type != t_s_typeless) {              \
            log_error("two or more data types in declaration" \
                      "specifiers\n");                        \
        }                                                     \
        ret_val.core_type = t_s_##KEYWORD;                    \
        continue;                                             \
    }

        XX(char);
        XX(int);
        XX(float);
        XX(double);
        XX(void);
#undef XX

        /* type-specifier - length modifiers */
        if (strcmp(t->str, "short") == 0) {
            if (ret_val.length_modifier != t_s_natural) {
                log_error("two 'length-modifiers' in declaration!"
                          " 'short' is not the only 'length-modifier'\n");
            }
            ret_val.length_modifier = t_s_short;
            continue;
        }
        if (strcmp(t->str, "long") == 0) {
            switch (ret_val.length_modifier) {
            case t_s_short:
                log_error("two 'length-modifiers' in declaration!"
                          " 'short' is not the only 'length-modifier'\n");
                continue;
            case t_s_natural: ret_val.length_modifier = t_s_long; continue;
            case t_s_long: ret_val.length_modifier = t_s_long_long; continue;
            case t_s_long_long:
                log_error("'long long long' is too long for dcc\n");
                continue;
            default:
                log_fatal("`length_modifier` enum error! please report!\n");
                exit(1);
            }
        }

#define XX(KEYWORD) /* type-specifier - signedness modifiers */ \
    if (strcmp(t->str, #KEYWORD) == 0) {                        \
        if (ret_val.signedness != t_s_none) {                   \
            log_error("both 'signed' and 'unsigned' in"         \
                      " declaration!\n");                       \
        }                                                       \
        ret_val.signedness = t_s_##KEYWORD;                     \
        continue;                                               \
    }

        XX(signed);
        XX(unsigned);
#undef XX

#define XX(KEYWORD) /* type-qualifier, function-specifier */ \
    if (strcmp(t->str, #KEYWORD) == 0) {                     \
        ret_val.is_##KEYWORD = true;                         \
        continue;                                            \
    }

        XX(const);
        XX(restrict);
        XX(volatile);
        XX(inline);
#undef XX

#define XX(KEYWORD) /* storage-class-specifier */ \
    if (strcmp(t->str, #KEYWORD) == 0) {          \
        ret_val.storage_class = s_c_##KEYWORD;    \
        s_c_so_far++;                             \
        continue;                                 \
    }

        XX(typedef);
        XX(extern);
        XX(static);
        XX(auto);
        XX(register);
#undef XX

        // at this point, we checked all declaration-specifiers.
        break;
    }
    // a few corrections to the type... implicit int:
    if (ret_val.core_type == t_s_typeless) {
        if (ret_val.signedness != t_s_none
            || ret_val.length_modifier != t_s_natural) {

            ret_val.core_type = t_s_int;
        } else {
            log_error("no core type deduced!\n");
            exit(1);
        }
    }
    // we must be out of this parsing phase. check the base type.
    if (s_c_so_far > 1) {
        log_error("two storage-specifiers in declaration!\n");
    }
    // a few unimplemented behavior checks
    if (ret_val.storage_class == s_c_typedef) {
        log_warn("'typedef' storage class is not supported\n");
    }
    if (ret_val.storage_class == s_c_extern) {
        log_warn("'extern' storage class is not supported\n");
    }

    return ret_val;
}

static inline const char* full_type_specifiers(struct decl_specifiers* ret_val) {
    static char buffer[4] = "";
    const char* ret;

    switch (ret_val->core_type) {
    case t_s_void: return "void";
    case t_s_double:
    case t_s_float: return ret_val->core_type == t_s_float ? "f32" : "f64";
    case t_s_char:
        memcpy(buffer,
               " 8\0\0", //\0 to keep it 4 bytes
               4); // 4 because that's optimized to 1 int equality
        break;
    case t_s_int:;
        const char* length = (const char*[]){
            [1 + t_s_short] = " 16", // the 1+ is to deal with the
            [1 + t_s_natural] = " 16", // negative index problem
            [1 + t_s_long] = " 32",
            [1 + t_s_long_long] = " 64",
        }[1 + ret_val->length_modifier];
        memcpy(buffer, length, 4);
        break;
    case t_s_typeless: // fallthrough
    default: log_error("bad type parsed\n");
    }
    buffer[0] = ret_val->signedness == t_s_unsigned ? 'u' : 'i'; // sign

    return buffer;
}

static inline struct decl_type get_array_decl(struct context* input) {
    // array-declaration :=
    // @       "[" [type-qualifier-or-static-list] expr(bp("=")) "]"
    //         "[" [type-qualifier-list] "*" "]"

    // however, the first "[" is already consumed.
#define ARRAY_QUALIFIER_AND_STATIC(XX) \
    XX(const)                          \
    XX(restrict)                       \
    XX(volatile)                       \
    XX(static)

#define XX(q) .is_##q = false,

    struct decl_type ret_val = {
        .t = d_array,
        ARRAY_QUALIFIER_AND_STATIC(XX)
        .size = NULL, // not yet decided
    };
    // now expecting one of:
    // - an expression with bp("=")
    // - list of type qualifiers/static
    // - nothing
    struct token* t = peek(input);
#undef XX
#define XX(q)                      \
    if (0 == strcmp(#q, t->str)) { \
        ret_val.is_##q = true;     \
        next(input);               \
        t = peek(input);           \
        continue;                  \
    }
    while (1) { // read type-qualifier-list
        ARRAY_QUALIFIER_AND_STATIC(XX)
        break;
    }
#undef XX
#undef ARRAY_QUALIFIER_AND_STATIC

    if (0 != strcmp("]", t->str)) { // not empty
        // get expression with bp("=") - which is lower than []
        ret_val.size = expr(
            bp(input, &(struct token){.str = strdup("=")}, infix), input);
    }
    // now expecting "]"
    expect_token("]", input);

    return ret_val;
}

static inline struct expr_ast*
    parse_assignment_expression(struct context* input) {
    static const struct token eq = {.str = "=", .t = t_punctuator};
    return expr(bp(input, &eq, infix), input);
}

static inline void parse_type_qualifier_list(struct decl_type* decl_so_far,
                                             struct context* input) {
    for (struct token* t = peek(input); t->t == t_keyword; t = peek(input)) {
#define XX(KEYWORD)                       \
    if (0 == strcmp(#KEYWORD, t->str)) {  \
        decl_so_far->is_##KEYWORD = true; \
        next(input);                      \
        continue;                         \
    }
        XX(const);
        XX(restrict);
        XX(volatile);
        break;
#undef XX
    }
}

static inline struct decl_type
    maybe_array_or_function_decarator(struct decl_type decl_so_far,
                                      struct decl_specifiers specs,
                                      struct context* input);

// given a `direct-declarator`, parse `direct-declarator[...]`
// array-declarator :=
// @         direct-declarator * "[" expr(bp("=")) "]"
//     |     direct-declarator * "[" type-qualifier-list expr(bp("=")) "]"
//     |     direct-declarator * "[" [type-qualifier-list] "]"
// @   |     direct-declarator * "[" static expr(bp("=")) "]"
//     |     direct-declarator * "[" static type-qualifier-list
//                                 expr(bp("=")) "]"
//     |     direct-declarator * "[" type-qualifier-list static
//                                 expr(bp("=")) "]"
//     |     direct-declarator * "[" [type-qualifier-list] "*" "]"
// we are at the *
static inline struct decl_type
    parse_array_declarator(struct decl_type decl_so_far,
                           struct decl_specifiers specs,
                           struct context* input) {
    expect_token("[", input);
    // for now the basic case: we are parsing "array of decl_so_far"
    // does not work with `int (*foo)[5];`
    struct decl_type ret = {.t = d_array,
                            .id = decl_so_far.id,
                            .is_const = false,
                            .is_restrict = false,
                            .is_volatile = false,
                            .is_static = false,
                            .size = NULL,
                            .declarator = xmalloc(sizeof ret)};
    decl_so_far.id = NULL;
#define GET_STATIC()                               \
    if (0 == strcmp("static", peek(input)->str)) { \
        ret.is_static = true;                      \
        next(input);                               \
    }

    GET_STATIC();
    parse_type_qualifier_list(&ret, input);
    GET_STATIC();

    struct token* t = peek(input);
    if (0 != memcmp("]", t->str, 2))
        ret.size = parse_assignment_expression(input);

    expect_token("]", input);

    *ret.declarator
        = maybe_array_or_function_decarator(decl_so_far, specs, input);

    return ret;
}

static inline void*
    x_list_one_or_more(const char* separator,
                       void parse_item(void* where, struct context*),
                       size_t item_size, size_t* count, struct context* input) {
    void* ret = malloc(item_size);
    *count = 1;
    parse_item(ret, input);
    while (strcmp(peek(input)->str, separator) == 0) {
        next(input);
        ++*count;
        xrealloc(ret, *count);
        parse_item(ret, input);
    }
    return ret;
}

// parameter-declaration:
// #         declaration-specifiers declarator
//           attribute-specifier-sequence declaration-specifiers declarator
//           [attribute-specifier-sequence] declaration-specifiers [abstract-declarator]
void parse_parameter_declaration(void* where, struct context* input) {
    struct c_var* ret = where;
    ret->specifiers = get_decl_specifiers(input);
    ret->t = get_declarator(input, ret->specifiers);
    ret->name = ret->t.id;

    ret->t.id = NULL;
}

// function-declarator:
//           direct-declarator "(" [parameter-type-list] ")"
// parameter-type-list:
// #         parameter-list
//           parameter-list "," "..."
//           "..."
// parameter-list:
// #         parameter-declaration
// #         parameter-list "," parameter-declaration
// parameter-declaration:
// #         declaration-specifiers declarator
//           attribute-specifier-sequence declaration-specifiers declarator
//           [attribute-specifier-sequence] declaration-specifiers [abstract-declarator]
// type-name:
//           specifier-qualifier-list [abstract-declarator]
// abstract-declarator:
//           pointer
//           [pointer] direct-abstract-declarator
static inline struct decl_type
    parse_function_declarator(struct decl_type decl_so_far,
                              struct decl_specifiers specs,
                              struct context* input) {
    struct decl_type ret = {.t = d_function,
                            .id = decl_so_far.id,
                            .ret_type = xmalloc(sizeof *ret.ret_type),
                            .argc = 0,
                            .argv = NULL};
    decl_so_far.id = NULL; // moved into ret
    *ret.ret_type = (struct c_var){
        .t = decl_so_far,
        .name = NULL,
        .specifiers = specs,
    };

    expect_token("(", input);

    ret.argv = x_list_one_or_more(",", parse_parameter_declaration,
                                  sizeof(struct c_var), &ret.argc, input);
    expect_token(")", input);
    return ret;
}

static inline void parse_attribute_specifier_sequence(struct context* input) {
    expect_token("[", input);
    expect_token("[", input);
    log_warn("attributes are not supported in dcc\n");
    struct token* t;
    for (t = peek(input); memcmp(t->str, "]", 2) != 0; t = peek(input))
        next(input);
    expect_token("]", input);
    expect_token("]", input);
}

// direct-declarator * "[" ...
// direct-declarator * "(" ...
// direct-declarator *
// we are at the *
static inline struct decl_type
    maybe_array_or_function_decarator(struct decl_type decl_so_far,
                                      struct decl_specifiers specs,
                                      struct context* input) {
    struct token* t = peek(input);
    if (t->str[0] == '[')
        return parse_array_declarator(decl_so_far, specs, input);
    else if (t->str[0] == '(')
        return parse_function_declarator(decl_so_far, specs, input);
    else
        return decl_so_far;
}

// declarator - the type so far
static inline struct decl_type
    get_direct_declarator(struct context* input, struct decl_type ret_type,
                          struct decl_specifiers specs);

struct decl_type get_declarator(struct context* input,
                                struct decl_specifiers specs) {
    // declarator :=
    // @         pointer direct-declarator
    // @         direct-declarator
    // pointer:
    // @         "*" [attribute-specifier-sequence] [type-qualifier-list]
    // @         "*" [attribute-specifier-sequence] [type-qualifier-list] pointer
    // @ = works. # = work in progress. @! = only simple case works.
    // as a first step, I would ignore attribute-specifiers, [[ ]]
    struct decl_type declarator = {.t = d_base, .id = NULL};

    for (struct token* t = peek(input); t->str[0] == '*'; t = peek(input)) {
        expect_token("*", input);
        struct decl_type* tmp = xmalloc(sizeof declarator);
        *tmp = declarator;
        declarator = (struct decl_type){.t = d_ptr,
                                        .id = NULL,
                                        .is_const = false,
                                        .is_restrict = false,
                                        .is_volatile = false,
                                        .is_static = false,
                                        .declarator = tmp};
        t = peek(input);
        if (t->t == t_keyword) parse_type_qualifier_list(&declarator, input);
        else if (t->t == t_punctuator) {
            parse_attribute_specifier_sequence(input);
        }
    }

    return get_direct_declarator(input, declarator, specs);
}

static inline struct decl_type
    get_direct_declarator(struct context* input, struct decl_type ret_type,
                          struct decl_specifiers specs) {
    // direct-declarator :=
    // @         identifier [attribute-specifier-sequence]
    // @!        "(" declarator ")"
    //           array-declarator [attribute-specifier-sequence]
    //           function-declarator [attribute-specifier-sequence]
    //
    // array-declarator :=
    // @         direct-declarator "[" [type-qualifier-list]
    //                                 [expr(bp("="))] "]"
    // @   |     direct-declarator "[" static [type-qualifier-list]
    //                                 expr(bp("=")) "]"
    // @   |     direct-declarator "[" type-qualifier-list static
    //                                 expr(bp("=")) "]"
    //     |     direct-declarator "[" [type-qualifier-list] * "]"
    //
    // function-declarator:
    //           direct-declarator "(" parameter-type-listopt ")"
    struct token* t = next(input);
    switch (t->t) {
    default: goto unexpected_token;
    case t_punctuator:
        if (memcmp("(", t->str, 2) == 0) {
            expect_token("(", input);
            ret_type = get_declarator(input, specs);
            expect_token(")", input);
            break;
        }
        goto unexpected_token;
    case t_identifier: ret_type.id = strdup(t->str); break;
    }
    return maybe_array_or_function_decarator(ret_type, specs, input);

unexpected_token:
    log_pos_error(stderr, input, t, "unexpected token %s\n", t->str);
    exit(1);
}

struct init_declaration_list parse_declaration(struct context* input) {
    // declaration :=
    // @         declaration-specifiers [init-declarator-list] ";"

    struct init_declaration_list ret_val = {
        .size = 0,
        .vars = NULL,
        .init_values = NULL,
    };
    // 1. read in the declarator-specifiers
    struct decl_specifiers specs = get_decl_specifiers(input);
    // 2. read init-declarator-list
    // init-declarator := declarator ["=" initializer]
    struct token* t;
    bool trailing_comma = false; // no comma encountered so far
    while (t = peek(input), t->t == t_identifier || strchr("*[(", *t->str)) {
        // declarators start with an identifier or or one of '*', '[', '('
        if (ret_val.size++ % 4 == 0) { // realloc the declaration
            size_t s = ret_val.size + 4;
            ret_val.vars = xrealloc(ret_val.vars, s * sizeof(*ret_val.vars));
            ret_val.init_values = xrealloc(ret_val.init_values,
                                           s * sizeof(*ret_val.init_values));
        }

        // populate the declaration
        struct decl_type declarator = get_declarator(input, specs);
        ret_val.vars[ret_val.size - 1] = (struct c_var){
            .specifiers = specs, .t = declarator, .name = declarator.id};
        ret_val.vars[ret_val.size - 1].t.id = NULL;

        trailing_comma = false;

        // get the name of the variable
        struct decl_type* d;
        for (d = &ret_val.vars[ret_val.size - 1].t;
             d->t != d_base && d->t != d_function;) {
            if (d->t == d_ptr || d->t == d_array) {
                d = d->declarator;
            } else {
                log_error("unknown decl type with ID=%d\n", (int)d->t);
            }
        }

        t = peek(input); // now it's one of [=,;] or an error
        if (0 == strcmp(t->str, "=")) {
            next(input);
            ret_val.init_values[ret_val.size - 1] = expr(
                bp(input, &(const struct token){.str = "=", .t = t_punctuator},
                   infix),
                input);
        } else {
            ret_val.init_values[ret_val.size - 1] = NULL;
        }

        if (t->t != t_punctuator) {
            log_pos_error(stderr, input, t,
                          "expected `=`, `;` or `,` before %s\n", t->str);
            exit(1);
        }
        switch (t->str[0]) {
        case ',':
            next(input);
            trailing_comma = true;
            continue; // this allows trailing ','! [STANDARD]
        case ';':
            // dont consume ';'
            // trailing_comma = false;
            continue;
        default:
            log_pos_error(stderr, input, t,
                          "two or more data types in declaration "
                          "specifiers\n");
        }
    }
    if (trailing_comma) log_error("trailing comma in declaration\n");

    expect_token(";", input);
    return ret_val;
}

static inline void print_decl_type(struct decl_type* d_t, int indent) {
    // assume we are at the start of the line
    switch (d_t->t) {
    case d_function:
        printf("function (\n");
        for (size_t arg = 0; arg < d_t->argc; arg++) {
            for (int i = 0; i < indent + 1; i++) printf("\t");
            struct init_declaration_list tmp
                = {.size = 1,
                   .vars = &d_t->argv[arg],
                   .init_values = (struct expr_ast*[]){NULL}};
            print_declaration(&tmp, indent + 1);
        }
        printf(")\n");
        for (int i = 0; i < indent; i++) printf("\t");
        printf("returning \n");
        for (int i = 0; i < indent + 1; i++) printf("\t");
        struct init_declaration_list tmp
            = {.size = 1,
               .vars = d_t->ret_type,
               .init_values = (struct expr_ast*[]){NULL}};
        print_declaration(&tmp, indent + 1);
        break;
    case d_array:
    case d_ptr:
        // reminder: this is saved in a linked list, of sorts, in reverse
        assert(d_t->declarator);
        if (d_t->declarator->t == d_ptr || d_t->declarator->t == d_array) {
            // print the list inverted
        } else if (d_t->declarator->t == d_base) {
            // just print the current thing and finish it
        } else {
            // idk how or what
            printf("function declaration - unsupported as of now!\n");
            break;
        }
        for (int i = 0; i < indent; i++) printf("\t");
        if (d_t->is_const) printf("const ");
        if (d_t->is_restrict) printf("restrict ");
        if (d_t->is_volatile) printf("volatile ");
        if (d_t->is_static) printf("static ");

        printf(d_t->t == d_array ? "array " : "pointer ");
        if (d_t->t == d_array && d_t->size) {
            printf("of size\n");
            print_expr_ast(d_t->size, indent + 1);
        }
        printf(d_t->t == d_array ? "of:\n" : "to:\n");
        for (int i = 0; i < indent; i++) printf("\t");
        print_decl_type(d_t->declarator, indent + 1);
        break;
    case d_base: break; // no point in printing the name
    }
}

void print_declaration(const struct init_declaration_list* decls, int indent) {
    for (size_t i = 0; i < decls->size; i++) {
        struct c_var* var = &decls->vars[i];
        // print name:
        if (var->name) printf("%s is ", var->name);
        // print specifiers
        struct decl_specifiers specs = var->specifiers;
        char* storage = (char*[]){
            [s_c_none] = "a ",
            [s_c_typedef] = "the `typedef` ",
            [s_c_extern] = "an `extern` symbol ",
            [s_c_static] = "`static` allocated ",
            [s_c_auto] = "a local (`auto`matic) variable ",
            [s_c_register] = "a `register` allocated variable ",
        }[specs.storage_class];
        printf("%s\n", storage);
        // read out the full type except the base type
        for (int level = 0; level < indent; level++) printf("\t");
        print_decl_type(&var->t, indent + 1);
        // now print base type
        if (var->specifiers.is_const) printf("const ");
        if (var->specifiers.is_restrict) printf("restrict ");
        if (var->specifiers.is_volatile) printf("volatile ");
        if (var->specifiers.is_inline) printf("inline ");

        // now to the base type- ([ui](8|16))|([uif](32|64))|void
        printf("%.4s ", full_type_specifiers(&var->specifiers));
        // print init values

        if (decls->init_values[i]) {
            printf("=\n");
            print_expr_ast(decls->init_values[i], indent + 1);
        }
        printf(";\n");
    }
}

void free_c_var(struct c_var* var) {
    if (var->name) free(var->name);
    var->name = NULL;

    struct decl_type* d_t = &var->t;

    if (d_t->id) free(d_t->id);
    d_t->id = NULL;

    switch (d_t->t) {
    case d_base: return; // nothing to do.
    case d_ptr:
    case d_array:
        if (d_t->size) free_expr_ast(d_t->size);
        d_t->size = NULL;

        struct c_var tmp = {
            .name = NULL,
            .t = *d_t->declarator,
        };
        free(d_t->declarator);
        d_t->declarator = NULL;
        free_c_var(&tmp);
        return;
    case d_function:
        for (size_t i = 0; i < d_t->argc; i++) free_c_var(&d_t->argv[i]);
		free(d_t->argv);
        free_c_var(d_t->ret_type);
		free(d_t->ret_type);
        d_t->ret_type = NULL;
        break;
    }
}
