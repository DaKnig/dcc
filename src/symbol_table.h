/*	this file holds and manages a global array of the symbols
	this includes vars, ptrs, funcs.
	this does not include a list of types - like structs, enums
	and unions. READ 6.2.3 in the 2017 standard.
 */
#include "type.h"
#include <stdbool.h>

struct symbol{
	struct type t;
	char* name;
	bool access;
};

void init_symbol_table(void);
	//this initializes the symbol table

unsigned add_to_table(struct symbol);
	//add this symbol to the table and the current frame
	//returns the index to the symbol table

void disable_symbol(unsigned index);
	//make symbol unaccessible, by index

long get_index(char* name);
	//returns the index of the symbol in the symbol table if exists
	//-1 if does not exist or not currently available in table
