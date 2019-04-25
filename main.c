#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "tokenizer.h"
#include "pratt.h"

//those are globals- because the whole program uses them
extern struct token* token_stream_head;
extern struct token* token_stream;
extern unsigned token_stream_len;


int main(){
	char str[50] = "5+6*9;";
	const char* ops[] = {"+","-","*","/"
						,"++","--","+=","-=","*=","/="
						,"|","&","^","~"
						,"|=","&=","^="
						,"||","&&","!"
						,"==","!=","<=",">=","<",">"
						,"(",")",";",",",":","?"
						,"[","]","{","}","->"};
	//do not change the above!

	printf("original: %s\n", str);

	token_stream_len = tokenize(&token_stream_head,str,ops,sizeof(ops)/sizeof(char*));
	token_stream=token_stream_head;

	printf("token stream:\n");
	print_tokens(token_stream_head,token_stream_len);

	struct ast* root = full_expression();
	printf("ast:\n");
	print_ast(root,0);

	free_ast(root);
	free_tokens(token_stream_head, token_stream_len);
	return 0;
}
