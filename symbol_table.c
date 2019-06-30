#include "type.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

struct symbol{
	struct type t;
	char* name;
	bool access;
};
struct symbol* symbol_table;
	//holds all symbols ever
unsigned max_table_size= 100;
unsigned table_size=0;

void init_symbol_table(){
	//this initializes the symbol table
	symbol_table = malloc(sizeof (struct symbol) * max_table_size);
}

unsigned add_to_table(struct symbol new_symbol){
	//add this symbol to the table and the current frame
	//returns the index to the symbol table
	if(table_size>=max_table_size){
		max_table_size *= 2;
		symbol_table = realloc(symbol_table, sizeof(struct symbol) * max_table_size);
		if (symbol_table == NULL)
			perror("Error - could not allocate the space for the symbol table");
	}
	symbol_table[table_size] = new_symbol;
	/*	TODO: add a check for wether another symbol with the same
		name already exists and is active in the current frame	*/
	symbol_table[table_size].access = true;
	return table_size++;
}

void disable_symbol(unsigned index){
	//make symbol unaccessible, by index
	if(index>=table_size)
		exit(fprintf(stderr,"Error - bad index [%u] for disabling a symbol",index));
	symbol_table[index].access=false;
}

long get_index(char* name){
	//returns the index of the symbol in the symbol table if exists
	//-1 if does not exist or not currently available in table
	for (unsigned i=0;i<table_size;i++)
		if (symbol_table[i].access && strcmp(symbol_table[i].name,name)==0)
			return i;
	return -1;
}
