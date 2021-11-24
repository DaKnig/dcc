#include <stdio.h>
#include <string.h>
#include "decl_parser.h"
#define DEBUG
#include "new_tokenizer.h"
#undef DEBUG

int main(void) {
    char* buffer[] = {
	"int* foo;",
	"int a,b",
	NULL
    };
    for (char** p=buffer; *p; p++) {
	struct context* input = create_ctx(fmemopen(*p, strlen(*p), "r"));
	// turns the string into a read-only file; makes a context out of it

	struct init_declaration_list* decls = parse_declaration(input);
	puts(*p);
	print_declaration(decls, 0);

	fclose(input->file); // close the string file
    }
    return 1;
}
