#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "type.h"
#include "util.h"
#include "symbol_table.h"


struct sym_scope* add_sym(struct sym_scope* table,struct symbol* new_sym){
    // inserts a copy of new_sym into the table, returns the table location
    // use like this- `t = insert_sym(t, s);`
    assert (new_sym && table);
    //sanity check
    for (unsigned i = 0; i<table->size; i++)
        if (table->sym_vector[i]->name == new_sym->name)
            return NULL;
        ///on repeating name, return a NULL

    table->size++;
    if (!(table->size & (table->size-1)))
        //realloc if power of 2
        table->sym_vector = xrealloc (table->sym_vector , 2*table->size*sizeof(*table->sym_vector));
    struct symbol* copy_of_new_sym = xmalloc(sizeof *new_sym);
    *copy_of_new_sym = *new_sym;
    table->sym_vector[table->size-1] = copy_of_new_sym;
    return table;
}

struct symbol* get_sym(struct sym_scope* table, const char* name){
	//returns the pointer to the symbol in the symbol table if exists, NULL otherwise
    assert(("how the hell did we get get a NULL name?",name));
    for ( ; table!=NULL ; table=table->parent)
        for (unsigned i=0 ; i<table->size ; i++)
            if (table->sym_vector[i]->name == name)
                return table->sym_vector[i];
    return NULL;
}

struct sym_scope* push_scope(struct sym_scope* table){
    //call this when entering a new scope
    //returns a ptr to the new scope
    struct sym_scope* temp = xmalloc(sizeof(*temp));
    *temp = (struct sym_scope){.parent = table, .sym_vector = NULL, .size = 0};
    return temp;
}

struct sym_scope* pop_scope(struct sym_scope* table){
    struct sym_scope* temp = table->parent;
    if (table->sym_vector != NULL)
        free(table->sym_vector);
    free(table);
    return temp;
}

void print_helper(FILE* out_file,struct sym_scope* table){
    //assume that it only goes 80 deep!
    if (table == NULL)
        return;
    print_helper(out_file,table->parent);
    unsigned depth;
    struct sym_scope* table_interator = table;
    for (depth = 0; table_interator!=NULL; table_interator=table_interator->parent, depth++);
    
    assert (depth<80);
    char indent_str[80]={0};
    memset(indent_str,' ',depth);

    for (unsigned i=0; i<table->size; i++)
        fprintf(out_file,"%s %s\n",indent_str,table->sym_vector[i]->name);
}

void print_scope_stack(FILE* out_file,struct sym_scope* table){
    //this is just for debugging

    print_helper(out_file,table);
}
