#include <stdlib.h> //NULL
#include <stdio.h>
#include "type.h"
#include "decl_parser.h"
#include "symbol_table.h"

int main(void){
	struct sym_scope* root = push_scope(NULL);

	struct type in={.general_type = var,.id = SPEC_INT};

	char name_arr[][3]={
		"x","y","z"
	};

	struct symbol sym_stream[]={
		{.t = in, .name=name_arr[0]},
		{.t = in, .name=name_arr[1]},
		{.t = in, .name=name_arr[2]},
	};
	{
		root = add_sym(root,&sym_stream[0]);
		root = add_sym(root,&sym_stream[1]);

		struct symbol* temp1 = get_sym(root, sym_stream[1].name);

		root = push_scope (root);
		root = add_sym(root,&sym_stream[1]);
		root = add_sym(root,&sym_stream[2]);

		struct symbol* temp2 = get_sym(root, sym_stream[1].name);

		fputs("first batch:\n",stderr);
		print_scope_stack(stderr,root);

		if (temp2==temp1)
			exit(fprintf(stderr,"for some reason the symbols collide"));
	}
	{
		struct sym_scope* temp = root;
		root = add_sym(root, &sym_stream[2]);
		if (root != NULL)
			exit(fprintf(stderr,"collision was not spotted"));
		root = temp;
	}
	{
		root = pop_scope(root);
		root = pop_scope(root);

		if (root != NULL)
			exit(fprintf(stderr,"collision was not spotted"));
	}

	return 0;
}
