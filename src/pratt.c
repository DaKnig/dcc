//implementation details:
//, is implemented as a binary term
//x,y,z parsed as (x,y),z and should be implemented properly!
//need to implement function pointers!
//need to implement cexpr_asts!

#include <stdio.h>
#include <stdlib.h> //exit
#include <assert.h>	//assert
#include <stddef.h>
#include <string.h> //strcmp
#include "pratt.h"
#include "tokenizer.h"

struct token* token_stream_head;
struct token* token_stream;
unsigned token_stream_len;

static inline int binding_power_formula(int x){
	//based on numbers from https://en.cppreference.com/w/c/language/operator_binding_power
	return (10*(16-x));
}

enum notation{prefix, infix};

static inline int infix_only(const struct token* token, enum notation n,
															int binding_power){
	switch(n){
		case prefix:
			fprintf(stderr,"expected an expressions before %s",
					token->str);
			exit (1);
		case infix:
			return  binding_power_formula(binding_power);
		default:	fprintf(stderr,"error: notation is neither infix nor prefix");
					exit(1);
	}
}
static inline int prefix_or_infix(enum notation n,int prefix_binding_power,
													int infix_binding_power){
	switch (n){
		case prefix:	return binding_power_formula(prefix_binding_power);
		case infix:		return binding_power_formula(infix_binding_power);
		default:		fprintf(stderr,"error: notation is not infix nor prefix");
						exit(1);
	}
}

int bp(const struct token* token, const enum notation n){
/*
	returns a binding power- how tightly does the operator bind to the operands
	near it	higher arguments to the auxilary functions (prefix_or_infix, 
	infix_only) == less bp a full expression is called with 16 so anything 
	higher is equally tight (including ;)
*/
//need to find out how to do ?:
	int ret_val;
	if(token == NULL){
		fprintf(stderr,"expected ';'");
		exit(1);
	}
	else if(strcmp(token->str, ";")==0 ||
		strcmp(token->str, ")")==0 ||
		strcmp(token->str, ":")==0)
		ret_val = prefix_or_infix(n,17,17);
	else if(strcmp(token->str, ",")==0)
		ret_val = infix_only(token,n,15);

	else if(strcmp(token->str,  "=")==0 ||
			strcmp(token->str, "+=")==0 ||
			strcmp(token->str, "-=")==0 ||
			strcmp(token->str, "*=")==0 ||
			strcmp(token->str, "/=")==0 ||
			strcmp(token->str, "%=")==0 ||
			strcmp(token->str,"<<=")==0 ||
			strcmp(token->str,">>=")==0 ||
			strcmp(token->str, "&=")==0 ||
			strcmp(token->str, "^=")==0 ||
			strcmp(token->str, "&=")==0)
		ret_val = infix_only(token,n,14);

	else if(strcmp(token->str, "||")==0)
		ret_val = infix_only(token,n,12);

	else if(strcmp(token->str, "&&")==0)
		ret_val = infix_only(token,n,11);

	else if(strcmp(token->str, "|")==0)
		ret_val = infix_only(token,n,10);

	else if(strcmp(token->str, "^")==0)
		ret_val = infix_only(token,n,9);

	else if(strcmp(token->str, "==")==0 ||
			strcmp(token->str, "!=")==0)
		ret_val = infix_only(token,n,7);

	else if(strcmp(token->str, ">>")==0 ||
			strcmp(token->str, "<<")==0)
		ret_val = infix_only(token,n,5);

	else if(strcmp(token->str,">" )==0 ||
			strcmp(token->str,"<" )==0 ||
			strcmp(token->str,">=")==0 ||
			strcmp(token->str,"<=")==0)
		ret_val = infix_only(token,n,6);

	else if(strcmp(token->str, "&")==0)
		ret_val = prefix_or_infix(n,2,8);

	else if(strcmp(token->str, "++")==0 ||
			strcmp(token->str, "--")==0)
		ret_val = prefix_or_infix(n,2,1);

	else if(strcmp(token->str, "!")==0 ||
			strcmp(token->str, "~")==0){
		if(n==prefix)
			ret_val = binding_power_formula(2);
		else if(n==infix){
			fprintf(stderr,"%s is out of place! didn't expected some"
			"expressions to it's left",	token->str);
			exit (1);		
		}
	}

	else if(strcmp(token->str, "*")==0)
		ret_val = prefix_or_infix(n,2,3);

	else if(strcmp(token->str, "/")==0 ||
			strcmp(token->str, "%")==0)
		ret_val = infix_only(token,n,3);

	else if(strcmp(token->str, "+")==0 ||
			strcmp(token->str, "-")==0)
		ret_val = prefix_or_infix(n,2,4);
	else if(strcmp(token->str, "?")==0)
		ret_val=infix_only(token,n,13);

	else{
		fprintf(stderr,"tried to evaluate the binding power of %s. failed"
													" miserably.", token->str);
		exit(1);
	}
	return ret_val;
}
static inline struct expr_ast* make_bin_tree(struct expr_ast* left,struct token* operator,
															struct expr_ast* right){
	if (right==NULL){
		fprintf(stderr, "encountered a NULL token. did you forget to add something to the end?");
		exit(1);
	}
	struct expr_ast* bin_tree = malloc( sizeof(struct expr_ast) );
	if (bin_tree == NULL){
		fprintf(stderr, "can't allocate memory for a binary tree");
		exit(1);
	}
	*bin_tree = (struct expr_ast) {
		.type = bin_op,
		.op = operator,
		.term[0] = left, .term[1] = right
	};
	return bin_tree;
}
static inline struct expr_ast* make_unary_tree(struct expr_ast* left,
														struct token* operator){
	struct expr_ast* unary_tree = make_bin_tree(left,operator,NULL);
	unary_tree->type = unary_op;
	return unary_tree;
}

static inline struct expr_ast* make_term_tree(struct token* token){
	struct expr_ast* tmp = malloc(sizeof(struct expr_ast));
	tmp->type = term;
	tmp->token = token;
	return tmp;
}

static inline struct token* peek(void){
	if(token_stream < token_stream_head + token_stream_len)
		return token_stream;
	else
		return NULL;
}
static inline struct token* next(void){
	if(token_stream < &token_stream_head[token_stream_len])
		return token_stream++;
	else
		return NULL;
}

void expect_token(const char* expected){
	//eat up the next token. crash if no match.
	char* delim = next()->str;
	if (strcmp(delim,expected)!=0){
		fprintf(stderr,"expected `%s` before `%s`",":",delim);
		exit(1);
	}
}

struct expr_ast* led(struct expr_ast* left, struct token* operator){
	if(operator->t == op){
		if( strcmp(operator->str,  "=")==0 ||
			strcmp(operator->str, "+=")==0 ||
			strcmp(operator->str, "-=")==0 ||
			strcmp(operator->str, "*=")==0 ||
			strcmp(operator->str, "/=")==0 ||
			strcmp(operator->str, "%=")==0 ||
			strcmp(operator->str,"<<=")==0 ||
			strcmp(operator->str,">>=")==0 ||
			strcmp(operator->str, "&=")==0 ||
			strcmp(operator->str, "^=")==0 ||
			strcmp(operator->str, "|=")==0)

			//right to left/right associative, infix/postfix operators
				return make_bin_tree(left,operator,expr(bp(operator,infix)-1));
		else if( strcmp(operator->str,"++")==0 ||
				strcmp(operator->str,"--")==0 ){
				operator->str[0] = 'p';
				//p+ = post increment
				return make_unary_tree(left, operator);
			}
		else if( strcmp(operator->str, "*")==0 ||
			strcmp(operator->str, "/")==0 ||
			strcmp(operator->str, "%")==0 ||
			strcmp(operator->str, "+")==0 ||
			strcmp(operator->str, "-")==0 ||

			strcmp(operator->str,"<<")==0 ||
			strcmp(operator->str,">>")==0 ||
			strcmp(operator->str, "|")==0 ||
			strcmp(operator->str, "&")==0 ||
			strcmp(operator->str, "^")==0 ||
			strcmp(operator->str,"||")==0 ||
			strcmp(operator->str,"&&")==0 ||

			strcmp(operator->str,"<" )==0 ||
			strcmp(operator->str,"<=")==0 ||
			strcmp(operator->str,">" )==0 ||
			strcmp(operator->str,">=")==0 ||
			strcmp(operator->str,"==")==0 ||
			strcmp(operator->str,"!=")==0 ||
			strcmp(operator->str, ",")==0)
			//left to right/left associative, infix/postfix operators
				return make_bin_tree(left,operator,expr(bp(operator,infix)));
		else if(strcmp(operator->str,"?")==0){
			struct expr_ast* middle = expr(0);
				assert (("expected something after the ?",middle!=NULL && peek()->str!=NULL));
			expect_token(":");
			struct expr_ast* right = expr(bp(operator,infix)-1);///////////testing
				assert (("expected something after the :",right!=NULL));
			struct expr_ast* ternary_tree = malloc(sizeof (struct expr_ast));
				assert (("malloc",ternary_tree!=NULL));
			*ternary_tree = (struct expr_ast) {
				.type = ternary_op,
				.op = operator,
				.term[0] = left, .term[1] = middle, .term[2] = right
			};
			return ternary_tree;
		}
		/*if nothing else is matched*/
		else{
			fprintf(stderr,"did not expect this op: `%s` with something to"
												" it's left\n", operator->str);
			exit(1);
		}
	}else{
		if(operator->t==identifier){
			fprintf(stderr,"did not expect this identifier here: %s\n", 
																operator->str);
			exit(1);
		}else if(operator->t==i_ltrl || operator->t==str_ltrl){
			fprintf(stderr,"did not expect this literal here: %s",
																operator->str);
			exit(1);
		}
		else{
			fprintf(stderr,"how did we get here? here's the OP that triggered"
													" me: %s", operator->str);
			exit(1);
		}
	}
}

struct expr_ast* make_comma_tree(void){
	struct token* delim;
	struct expr_ast* temp = malloc (sizeof (struct expr_ast));
	*temp=(struct expr_ast){
		.type =comma,
		.argc =0,
		.argv =NULL,
		.func_name =NULL
	};
	do{
		temp->argv=realloc(temp->argv, sizeof(struct expr_ast[++temp->argc]));
		if (temp->argv==NULL){
			fprintf(stderr,"failed to realloc the comma expression");
			exit(1);
		}
		temp->argv[temp->argc-1] = expr(bp(&(struct token){.str = strdup(",")}, infix));
		delim = peek();
	}while(strcmp(delim->str,",")==0 && (next(),1));
	return temp;
}

struct expr_ast* nud(struct token* token){
	struct expr_ast* ret_val;
	if(token->t == i_ltrl || token->t == str_ltrl)
		ret_val= make_term_tree(token);

	else if(token->t == identifier){
		if (strcmp (peek()->str, "(") != 0)
			ret_val= make_term_tree(token);			
		else{			//do function call stuff
			next();//eat up the '('
			struct expr_ast* temp = make_comma_tree();
			{//eat up the ')'
				char* delim=next()->str;
				if (strcmp(delim,")")!=0){
					fprintf(stderr,"expected `%s` before `%s`", ")",delim);
					exit(1);
				}
			}

			temp->func_name = token;
			temp->type=func_call;
			ret_val= temp;
		}
	}

	//parse this operator
	else if(strcmp(token->str,"+" )==0 ||
	   strcmp(token->str,"-" )==0 ||
	   strcmp(token->str,"!" )==0 ||
	   strcmp(token->str,"~" )==0 ||
	   strcmp(token->str,"*" )==0 ||
	   strcmp(token->str,"&" )==0 ||
	   strcmp(token->str,"++")==0 ||
	   strcmp(token->str,"--")==0)
		return make_unary_tree(expr(bp(token,prefix) - 1) , token);
	else if(strcmp(token->str,"(")==0){
	//get expression , expect it to return with a ')'
		struct expr_ast *temp = expr(0);
		expect_token(")");
		ret_val=temp;
		//maybe later add a field to the `(` that contains pointer to `)`
	}
	else{
		fprintf(stderr,"expected expression before: '%s'", token->str);
		exit(1);
	}
	return ret_val;
}

struct expr_ast* expr(int rbp){
	if (peek()==NULL){
		return NULL;
	}
	struct expr_ast* left = nud(next());
	while (peek()!=NULL&&bp(peek(),infix) > rbp)
		left = led(left, next());
	return left;
}

struct expr_ast* full_expression(void){
	return expr(0);
}
static inline void print_indent(int i){
	for (;i>0;i--)
		printf("|   ");
	printf("-> ");
}

void print_expr_ast(struct expr_ast* root, int indent){
	print_indent(indent);
	switch (root->type){
		case term:
			print_tokens(root->token, 1);
			break;

		case ternary_op:
			print_tokens(root->op, 1);
			for(int i=0;i<3;i++)
				print_expr_ast(root->term[i], indent+1);
			break;

		case bin_op:
			print_tokens(root->op, 1);
			for(int i=0;i<2;i++)
				print_expr_ast(root->term[i], indent+1);
			break;

		case unary_op:
			print_tokens(root->op, 1);
			print_expr_ast(root->term[0], indent+1);
			break;

		case func_call:
			print_tokens(root->func_name, 1);
			for (int i=0; i<root->argc; i++){
				print_expr_ast(root->argv[i],indent+1);
			}
			break;

		case comma:
			printf("comma expression here. not implemented yet.");
			break;

		case unprocessed:
			printf("unprocessed expression here. not implemented yet.");
			break;
	}
}

void free_expr_ast(struct expr_ast* root){
	switch (root->type){
		case term:
			break;
		case ternary_op:
			free_expr_ast(root->term[2]);	//fallthrough
		case bin_op:
			free_expr_ast(root->term[1]);	//fallthrough
		case unary_op:
			free_expr_ast(root->term[0]);
			break;
		case func_call:
		case comma:
			for (int i=0; i<root->argc;i++){
				free_expr_ast(root->argv[i]);
			}
			free(root->argv);
			break;
		case unprocessed:
			fprintf(stderr,"encountered an unprocessed tree while freeing");
			break;		
	}
	free(root);
}
