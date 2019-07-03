#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

enum token_type{
		op, i_ltrl, str_ltrl, identifier,
};
struct token{
	char* str;
	enum token_type t;
};

void add_token(struct token** tokens, char* new_token, unsigned *arr_size,
						unsigned *arr_size_max, enum token_type t){
	if ((*arr_size)>=*arr_size_max){
		*arr_size_max *= 2;
		*tokens = realloc(*tokens, *arr_size_max * sizeof(struct token));
		if (*tokens == NULL)
			printf("out of memory for tokens\n requested size: %ld\n",
					*arr_size_max * sizeof(char*));
	}
	(*tokens)[*arr_size].t = t;
	(*tokens)[*arr_size].str = new_token;
	if ((*tokens)[*arr_size].str == NULL)
			printf("out of memory for the string of the token.\n the token was: \"%s\"\n",
					new_token);
	(*arr_size)++;
}

int find_op(char** ops, int ops_n, char* str){
	int i;
	size_t max_len=0;
	long best_candidate_index=-1;
	for (i=0;i<ops_n;i++){
		if (strstr(str,ops[i]) == str && strlen(ops[i]) > max_len){
			max_len = strlen(ops[i]);
			best_candidate_index = i;
		}
	}
	return best_candidate_index;
}

unsigned tokenize(struct token** tokens, char* str, char** ops, int ops_n){
	unsigned arr_size=0;
	unsigned max_size=50;
	*tokens = (struct token*) calloc(max_size, sizeof(struct token));
	if (*tokens == NULL) printf("can't allocate tokens\n");
	char tmp[50]={0};//add more later if needed
	while(str[0] != '\0'){
		if (isspace(str[0])){
			str++;
			continue;
		}
		if (find_op(ops,ops_n,str) != -1){
			int op_identifier=find_op(ops,ops_n,str);
			add_token(tokens, strdup(ops[op_identifier]), &arr_size, &max_size, op);
			str+=strlen(ops[op_identifier]);
			continue;
		}
		if (isdigit(str[0])){
			//add hex support later
			//add float support
			if(str[0]=='0' && str[1] == 'x'){
				strcpy(tmp,"0x");
				str+=2;
			}
			unsigned i=0;
			for (i=0 ;isdigit(str[0]) && (i+1)<sizeof(tmp); str++){
				tmp[i++]=str[0];
			}
			tmp[i]='\0';
			if(i+1>=sizeof(tmp))   
				fprintf(stderr,"the number starting with %s is too big",tmp);
			
			add_token(tokens, strdup(tmp), &arr_size, &max_size, i_ltrl);
			continue;
		}
		if(str[0]=='\"'){
			unsigned i=0;
			do{
				if(str[0] == '\\' && str[1]!='\0' && strchr("\n\'\"\\",str[1]) != NULL){
					//implement this later: https://en.wikipedia.org/wiki/Escape_sequences_in_C
					//basically, just "if \ then get integer"
					tmp[i++]=str[1];
					str+=2;
					continue;
				}
				tmp[i++]=str[0];
				str++;
			}while(str[0]!='\"' && str[0]!='\0' && i+2<sizeof(tmp));
			if(i>=sizeof(tmp))
				fprintf(stderr,"the sting starting with \" %s \" is too long\n",tmp);
			tmp[i++]=str[0]; tmp[i]='\0';
			str++;
			add_token(tokens, strdup(tmp), &arr_size, &max_size, str_ltrl);
			continue;
		}
		if(isalnum(str[0]) || str[0]=='_'){//is identifier. identifiers consist of alphanumeric chars or '_'s
			unsigned i=0;
			while( (isalnum(str[0]) || str[0]=='_') && (i+1)<sizeof(tmp)){
				tmp[i++]=str[0];
				str++;
			}
			tmp[i]='\0';
			if(i+1>=sizeof(tmp))
			   fprintf(stderr,"the identifier starting with %s is too big",tmp);
			add_token(tokens, strdup(tmp), &arr_size, &max_size, identifier);
			continue;
		}
		printf("unexpected char: %c while trying to parse: %s\n",str[0],&str[-1]);
		str++;
	}
	return arr_size;
}

void print_tokens(struct token* tokens, unsigned n){
	for (unsigned i=0; i<n; i++)
		puts(tokens[i].str);
}
void free_tokens(struct token* token_stream_head, unsigned token_stream_len){
	for (unsigned i=0;i<token_stream_len;i++){
		free(token_stream_head[i].str);
	}
	free (token_stream_head);
}
