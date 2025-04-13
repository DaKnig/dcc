#ifndef DECL_PARSER
#define DECL_PARSER

/*
  this is a parser for an alternative C declaration syntax, described here:
  https://gist.github.com/aaaaaa123456789/96f1163c4672af55bb8cec5ca026839c
  specifically, the following definitions changed:
  - abstract-declarator
  - abstract-array-declaration
  - abstract-function-declaration
  - (maybe internal) array-declaration
  - (maybe internal) function-declaration

  the following do not get changed:
  - declaration
  - declaration-specifiers
  - init-declarator (which calls declarator)
  - type-name
  - specifier-qualifier-list
  - parameter-type-list
  - parameter-list
  - parameter-declaration

  might be replaced later with standard C declaration syntax.

  the base of the system is the type system, the type struct.
*/

#include "tokenizer.h"
#include "util.h"

#include <stdbool.h>
#include <stdint.h>

struct c_var;
struct c_func;

struct decl_type {
  enum { d_base, d_ptr, d_array, d_function } t;

  char *id;

  union {
    struct {                                   // d_ptr/d_array
      bool is_const, is_restrict, is_volatile; // type-qualifier-list
      bool is_static;                          // for array size
      struct expr_ast *size;                   // for array size
      struct decl_type *declarator;            // recursive
    };
    struct { // d_function
      struct c_var *ret_type;
      size_t argc;        // number of args
      struct c_var *argv; // the args
    };
  };
};

struct decl_specifiers {
  enum {
    s_c_none,
    s_c_typedef,
    s_c_extern,
    s_c_static,
    s_c_auto,
    s_c_register,
  } storage_class;                                    // only one may appear
  bool is_const, is_restrict, is_volatile, is_inline; // any combo of...

  enum { t_s_none, t_s_signed, t_s_unsigned } signedness;
  enum { t_s_short = -1, t_s_natural, t_s_long, t_s_long_long } length_modifier;
  enum {
    t_s_typeless,
    t_s_char,
    t_s_int,
    t_s_float,
    t_s_double,
    t_s_void
  } core_type;
};

struct c_var {
    struct decl_specifiers specifiers; // the base type (after all pointers)
    struct decl_type t;
    char* name; // if NULL, derives from t
};

struct init_declaration_list {
  // every declaration line declares multiple vars...
  size_t size; // how many declarations are in the list?
  struct c_var *vars;
  struct expr_ast **init_values; // with matching indexes; NULL if none
};

void print_declaration(const struct init_declaration_list *d, int indent);
struct init_declaration_list parse_declaration(struct context *input);

struct decl_specifiers get_decl_specifiers(struct context *input);
struct decl_type get_declarator(struct context *input, struct decl_specifiers);

void free_c_var(struct c_var *var);

#endif
