#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "new_tokenizer.h"
#include "pratt.h"
#include "statement_parser.h"
//those are globals- because the whole program uses them
//extern struct token* token_stream_head;
//extern struct token* token_stream;
//extern unsigned token_stream_len;


int main(void){
	char str[50000] = "{5+6*9;}";

	printf("original: %s\n", str);

	// token_stream_len = tokenize(&token_stream_head,str,ops,sizeof(ops)/sizeof(char*));
	// token_stream=token_stream_head;

	// printf("token stream:\n");
	// print_tokens(token_stream_head,token_stream_len);

//	struct expr_ast* root = full_expression();
//	printf("ast:\n");
//	print_expr_ast(root,0);
	struct context* input=create_ctx(stdin);
	do{
		// token_stream_len = tokenize(&token_stream_head,str,ops,sizeof(ops)/sizeof(char*));
		// token_stream=token_stream_head;

		// printf("token stream:\n");
		// print_tokens(token_stream_head,token_stream_len);
		struct statement* s=parse_statement(input);
		puts("statement tree:");
		print_statement(s,0);
		char* ptr;
		for (ptr=str; !feof(stdin) && ptr<str+40000; ptr++) {
			*ptr=getchar();
			if (*ptr == '\b')
				ptr--;
		}
		*ptr='\0';
		clearerr(stdin);
	} while (1);
//	free_expr_ast(root);
	// free_tokens(token_stream_head, token_stream_len);
	return 0;
}
