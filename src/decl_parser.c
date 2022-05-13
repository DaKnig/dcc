#include "decl_parser.h"

#include "assert.h"
#include "log.h"
#include "pratt.h"
#include "util.h"

#include <stdbool.h>

static struct decl_specifiers* get_decl_specifiers(struct context* input) {
    // syntax described at docs/declaration-specifier.dot
    struct decl_specifiers* ret_val = xmalloc(sizeof *ret_val);
    *ret_val = (struct decl_specifiers){
        .storage_class = s_c_none,

        .is_const = false,
        .is_restrict = false,
        .is_volatile = false,
        .is_inline = false,

        .full_type_specifiers = "",

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
        if (ret_val->core_type != t_s_typeless) {             \
            log_error("two or more data types in declaration" \
                      "specifiers\n");                        \
        }                                                     \
        ret_val->core_type = t_s_##KEYWORD;                   \
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
            if (ret_val->length_modifier != t_s_natural) {
                log_error("two 'length-modifiers' in declaration!"
                          " 'short' is not the only 'length-modifier'\n");
            }
            ret_val->length_modifier = t_s_short;
            continue;
        }
        if (strcmp(t->str, "long") == 0) {
            switch (ret_val->length_modifier) {
            case t_s_short:
                log_error("two 'length-modifiers' in declaration!"
                          " 'short' is not the only 'length-modifier'\n");
                continue;
            case t_s_natural: ret_val->length_modifier = t_s_long; continue;
            case t_s_long: ret_val->length_modifier = t_s_long_long; continue;
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
        if (ret_val->signedness != t_s_none) {                  \
            log_error("both 'signed' and 'unsigned' in"         \
                      " declaration!\n");                       \
        }                                                       \
        ret_val->signedness = t_s_##KEYWORD;                    \
        continue;                                               \
    }

        XX(signed);
        XX(unsigned);
#undef XX

#define XX(KEYWORD) /* type-qualifier, function-specifier */ \
    if (strcmp(t->str, #KEYWORD) == 0) {                     \
        ret_val->is_##KEYWORD = true;                        \
        continue;                                            \
    }

        XX(const);
        XX(restrict);
        XX(volatile);
        XX(inline);
#undef XX

#define XX(KEYWORD) /* storage-class-specifier */ \
    if (strcmp(t->str, #KEYWORD) == 0) {          \
        ret_val->storage_class = s_c_##KEYWORD;   \
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
    if (ret_val->core_type == t_s_typeless) {
        if (ret_val->signedness != t_s_none
            || ret_val->length_modifier != t_s_natural) {

            ret_val->core_type = t_s_int;
        } else {
            log_error("no core type deduced! returning NULL\n");
            return NULL;
        }
    }
    // we must be out of this parsing phase. check the base type.
    if (s_c_so_far > 1) {
        log_error("two storage-specifiers in declaration!\n");
    }
    // a few unimplemented behavior checks
    if (ret_val->storage_class == s_c_typedef) {
        log_warn("'typedef' storage class is not supported\n");
    }
    if (ret_val->storage_class == s_c_extern) {
        log_warn("'extern' storage class is not supported\n");
    }

    // construct the full type
    ret_val->full_type_specifiers[0]
        = ret_val->signedness == t_s_unsigned ? 'u' : 'i'; // sign

    switch (ret_val->core_type) {
    case t_s_void: memcpy(ret_val->full_type_specifiers, "void", 4); break;
    case t_s_double:
    case t_s_float:
        memcpy(ret_val->full_type_specifiers,
               ret_val->core_type == t_s_float ? "f32" : "f64", 4);
        break;
    case t_s_char:
        memcpy(ret_val->full_type_specifiers,
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
        memcpy(ret_val->full_type_specifiers, length, 4);
        break;
    case t_s_typeless: // fallthrough
    default: log_error("bad type parsed\n");
    }
    if (ret_val->full_type_specifiers[0] == ' ')
        ret_val->full_type_specifiers[0]
            = ret_val->signedness == t_s_unsigned ? 'u' : 'i'; // sign for ints

    assert(strchr("uifv", ret_val->full_type_specifiers[0]));

    return ret_val;
}

static inline struct decl_type get_declarator(struct context* input) {
    // declarator :=
    // @         identifier
    //           identifier array-declaration
    //           identifier function-declarator
    //
    // @         * [type-qualifier-list] declarator
    //           array-declaration declarator
    //           "(" parameter-type-list ")" declarator

    // that's still too complex. what's implemented would be marked with @
    struct token* t = next(input);
    struct decl_type ret_val;
    switch (t->t) {
    // non-recursive versions
    case t_identifier:
        ret_val = (struct decl_type){
            // base case...
            .t = d_base,
            .name = strdup(t->str),
        };
        // code for types 2-3
        t = peek(input);
        if (t->str[0] == '[' || t->str[0] == '(') {
            log_pos_error(stderr, input, t,
                          "declarator types 2, 3 not supported yet.\n");
            exit(1);
        }
        break;
    case t_punctuator:
        // strcmp required for long punctuators
        if (0 == strcmp("*", t->str)) {
            ret_val = (struct decl_type){
                .t = d_ptr,
                .is_const = false,
                .is_restrict = false,
                .is_volatile = false,
            };

            while (peek(input)->t == t_keyword) {
                t = next(input);
                if (0 == strcmp(t->str, "const")) {
                    ret_val.is_const = true;
                } else if (0 == strcmp(t->str, "restrict")) {
                    ret_val.is_restrict = true;
                } else if (0 == strcmp(t->str, "volatile")) {
                    ret_val.is_volatile = true;
                } else {
                    log_error("unexpected token %s in pointer declarator\n",
                              t->str);
                }
            }
            ret_val.declarator = xmalloc(sizeof *ret_val.declarator);
            *ret_val.declarator = get_declarator(input);
            break;
        } else if (0 == strcmp("[", t->str)) {
            // [] before identifier
            ret_val = (struct decl_type){
                .t = d_array,
                .is_const = false,
                .is_restrict = false,
                .is_volatile = false,
                .is_static = false,
                .size = NULL, // not yet decided
            };
            // now expecting one of:
            // - an expression with bp("=")
            // - list of type qualifiers/static (UNIMPLEMENTED)
            // - nothing
            t = peek(input);
            if (0 != strcmp("]", t->str)) { // not empty
                // get expression with bp("=") - which is lower than []
                ret_val.size = expr(
                    bp(input, &(struct token){.str = strdup("=")}, infix),
                    input);
            }
            // now expecting "]"
            expect_token("]", input);
            ret_val.declarator = xmalloc(sizeof *ret_val.declarator);
            *ret_val.declarator = get_declarator(input);
            break;
        } else if (0 == strcmp("(", t->str)) {
            //TODO - impl ()
            log_error("functions are not supported yet\n");
            exit(1);
        } else {
            log_error("expected identifier, '*', '[' or '(' before '%s'\n",
                      t->str);
            exit(1);
        }
        break;
    case t_string:
    case t_char:
    case t_float:
    case t_keyword: //fallthrough
    case t_EOF:
    case t_int:
    case t_bad_token:
    case t_unknown: //fallthrough
    default:
        log_error("expected identifier, '[' or '(' before '%s'\n", t->str);
        exit(1);
    }
    return ret_val;
}

struct init_declaration_list* parse_declaration(struct context* input) {
    // declaration := declaration-specifiers [init-declarator-list] ";"

    struct init_declaration_list* ret_val = xmalloc(sizeof *ret_val);
    *ret_val = (struct init_declaration_list){
        .size = 0,
        .vars = NULL,
        .init_values = NULL,
    };
    // 1. read in the declarator-specifiers
    struct decl_specifiers* specs = get_decl_specifiers(input);
    // 2. read init-declarator-list
    // init-declarator := declarator ["=" initializer]
    struct token* t;
    bool trailing_comma = false; // no comma encountered so far
    while (t = peek(input), t->t == t_identifier || strchr("*[(", *t->str)) {
        // declarators start with an identifier or or one of '*', '[', '('
        if (ret_val->size++ % 4 == 0) { // realloc the declaration
            size_t s = ret_val->size + 4;
            ret_val->vars = xrealloc(ret_val->vars, s * sizeof(*ret_val->vars));
            ret_val->init_values = xrealloc(ret_val->init_values,
                                            s * sizeof(*ret_val->init_values));
        }

        // populate the declaration
        ret_val->vars[ret_val->size - 1] = (struct c_var){
            .specifiers = *specs,
            .t = get_declarator(input),
            .name = NULL,
        };
        // get the name of the variable
        struct decl_type* d;
        for (d = &ret_val->vars[ret_val->size - 1].t;
             d->t != d_base && d->t != d_function;) {
            if (d->t == d_ptr || d->t == d_array) {
                d = d->declarator;
            } else {
                log_error("unknown decl type with ID=%d\n", (int)d->t);
            }
        }
        ret_val->vars[ret_val->size - 1].name = d->name;

        t = peek(input); // now it's one of [=,;] or an error
        if (0 == strcmp(t->str, "=")) {
            next(input);
            &(const struct token){.str = "=", .t = t_punctuator};
            ret_val->init_values[ret_val->size - 1] = expr(
                bp(input, &(const struct token){.str = "=", .t = t_punctuator},
                   infix),
                input);
        } else {
            ret_val->init_values[ret_val->size - 1] = NULL;
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
    free(specs);
    if (trailing_comma) log_error("trailing comma in declaration\n");

    expect_token(";", input);
    return ret_val;
}

static inline void print_decl_type(struct decl_type* d_t, int indent) {
    // assume we are at the start of the line
    for (int i = 0; i < indent; i++) printf("\t");
    switch (d_t->t) {
    case d_function:
        printf("function declaration - unsupported as of now!\n");
        break;
    case d_array:
    case d_ptr:
        if (d_t->is_const) printf("const ");
        if (d_t->is_restrict) printf("restrict ");
        if (d_t->is_volatile) printf("volatile ");
        if (d_t->is_static) printf("static ");

        printf(d_t->t == d_array ? "array " : "pointer ");
        if (d_t->t == d_array && d_t->size) {
            printf("of size \n");
            print_expr_ast(d_t->size, indent + 1);
        }

        assert(d_t->declarator);
        printf(d_t->t == d_array ? "of:\n" : "to:\n");
        print_decl_type(d_t->declarator, indent + 1);
        break;
    case d_base: break; // no point in printing the name
    }
}

void print_declaration(struct init_declaration_list* decls, int) {
    for (size_t i = 0; i < decls->size; i++) {
        struct c_var* var = &decls->vars[i];
        // print name:
        printf("%s is ", var->name);
        /*    print_decl_type(struct decl_type* d_t, int indent)    */
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
        print_decl_type(&var->t, 1);
        // now print base type
        if (var->specifiers.is_const) printf("const ");
        if (var->specifiers.is_restrict) printf("restrict ");
        if (var->specifiers.is_volatile) printf("volatile ");
        if (var->specifiers.is_inline) printf("inline ");

        // now to the base type- ([ui](8|16))|([uif](32|64))|void
        printf("%.4s ", var->specifiers.full_type_specifiers);
        // print init values

        if (decls->init_values[i]) {
            printf("=\n");
            print_expr_ast(decls->init_values[i], 1);
        }
        printf(";\n");
    }
}
