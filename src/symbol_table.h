#ifndef SYMTABLE_H
#define SYMTABLE_H
/*	this file implements a symbol table, which consists of a scope stack
	this includes vars, ptrs, funcs.
	this does not include a list of types - like structs, enums	and unions.
	READ 6.2.3 in the 2017 standard.
 */
#include <stdbool.h>
#include <stdio.h>
#include "type.h"

struct symbol{
    struct type t;
    char* name; //should be capped by the lexer . assumes that it is processed by atom.c beforehand
};

struct sym_scope{
    struct sym_scope* parent;   //NULL if global
    struct symbol** sym_vector; //all the symbols in that sym_scope
    unsigned size;          //the number of symbols in the vector
};

struct sym_scope* add_sym(struct sym_scope* table,struct symbol* new_sym);
    // inserts a copy of new_sym into the table, returns the table location
    // use like this- `t = insert_sym(t, s);`

struct symbol* get_sym(struct sym_scope* table, const char* name);
	//returns the pointer to the symbol in the symbol table if exists, NULL otherwise

struct sym_scope* push_scope(struct sym_scope* table);
    //make a new scope and push it to the stack
	//push NULL to init the global scope
	//call this when entering a new scope
    //returns a ptr to the new scope

struct sym_scope* pop_scope(struct sym_scope* table);
	//deletes the scope
	//call this when exiting the scope
	//returns the pointer to the parent

void print_scope_stack(FILE* out_file,struct sym_scope* table);
	//prints the scope stack. mainly for debugging.

#endif
