//implementation details:
//, is implemented as a binary term
//x,y,z parsed as (x,y),z and should be implemented properly!
#include <stdio.h>
#include <stdlib.h> //exit
#include <stddef.h>
#include <string.h> //strcmp

#include "tokenizer.h"

struct token* token_stream_head;
struct token* token_stream;
int token_stream_len;

enum ast_type
{
	term,
	unary_op,
	bin_op,
	ternary_op,
	comma,
	func_call,
	unprocessed
};

struct ast{
	enum ast_type type;
	union{
		struct{//term
			struct token* token;
		};
		struct{//op - unary, binary, ternary
			struct token* op;
			struct ast* term[3];
		};
		struct{//func call, comma
			struct token* func_name;
			struct ast* *argv;
			int argc;
			//pointer to a block
		};
	};
};
struct ast* expr(int bp);

static inline int binding_power_formula(int x){
	//based on numbers from https://en.cppreference.com/w/c/language/operator_binding_power
	return (10*(16-x));
}

enum notation{prefix, infix};

static inline int infix_only(const struct token* token, enum notation n, int binding_power){
	switch(n){
		case prefix:
			fprintf(stderr,"expected an expressions before %s",
					token->str);
			exit (1);		
		case infix:
			return  binding_power_formula(binding_power);
		default:	fprintf(stderr,"error: notation is not infix nor prefix");
					exit(1);
	}
}
static inline int prefix_or_infix(enum notation n,int prefix_binding_power,int infix_binding_power){
	switch (n){
		case prefix:	return binding_power_formula(prefix_binding_power);
		case infix:		return binding_power_formula(infix_binding_power);
		default:		fprintf(stderr,"error: notation is not infix nor prefix");
						exit(1);
	}
}

int bp(const struct token* token, const enum notation n){
/*
	returns a binding power- how tightly does the operator bind to the operands near it
	higher arguments to the auxilary functions (prefix_or_infix, infix_only) == less bp
	a full expression is called with 16 so anything higher is equally tight (including ;)
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
		ret_val = infix_only(token,n,14);

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
			fprintf(stderr,"%s is out of place! didn't expected some expressions to it's left",
					token->str);
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
	
	else{
		fprintf(stderr,"tried to evaluate the binding power of %s. failed miserably.",
					token->str);
		exit(1);
	}
	return ret_val;
}

static inline struct ast* make_bin_tree(struct ast* left,struct token* operator,struct ast* right){
	struct ast* bin_tree = malloc( sizeof(struct ast) );
	if (bin_tree == NULL){
		fprintf(stderr, "can't allocate memory for a binary tree");
		exit(1);
	}
	*bin_tree = (struct ast) {
		.type = bin_op,
		.op = operator,
		.term[0] = left, .term[1] = right
	};
	return bin_tree;
}

static inline struct ast* make_unary_tree(struct ast* left, struct token* operator){
	struct ast* unary_tree = make_bin_tree(left,operator,NULL);
	unary_tree->type = unary_op;
	return unary_tree;
}

static inline struct ast* make_term_tree(struct token* token){
	struct ast* tmp = malloc(sizeof(struct ast));
	tmp->type = term;
	tmp->token = token;
	return tmp;
}

struct ast* led(struct ast* left, struct token* operator){
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
		if( strcmp(operator->str,"++")==0 ||
			strcmp(operator->str,"--")==0 ){
				operator->str[0] = 'p';
				//p+ = post increment
				return make_unary_tree(left, operator);
			}
		if( strcmp(operator->str, "*")==0 ||
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
		/*if nothing else is matched*/{
			fprintf(stderr,"did not expect this op: \"%s\" with something to it's left\n", 
					operator->str);
			exit(1);
		}
	}else{
		if(operator->t==identifier){
			fprintf(stderr,"did not expect this identifier here: %s\n", operator->str);
			exit(1);
		}else if(operator->t==i_ltrl || operator->t==str_ltrl){
			fprintf(stderr,"did not expect this literal here: %s", operator->str);
			exit(1);
		}
		else{
			fprintf(stderr,"how did we get here? here's the OP that triggered me: %s", operator->str);
			exit(1);
		}
	}
}

static inline struct token* peek(){
	if(token_stream < token_stream_head + token_stream_len)
		return token_stream;
	else
		return NULL;	
}
static inline struct token* next(){
	if(token_stream < &token_stream_head[token_stream_len])
		return token_stream++;
	else
		return NULL;	
}

struct ast* make_comma_tree(){
	struct token* delim;
	struct ast* temp = malloc (sizeof (struct ast));
	*temp=(struct ast){
		.type =comma,
		.argc =0,
		.argv =NULL,
		.func_name =NULL
	};
	do{
		temp->argv=realloc(temp->argv, sizeof(struct ast[++temp->argc]));
		if (temp->argv==NULL){
			fprintf(stderr,"failed to realloc the comma expression");
			exit(1);
		}
		temp->argv[temp->argc-1] = expr(bp(&(struct token){.str = ","}, infix));
		delim = peek();
	}while(strcmp(delim->str,",")==0 && (next(),1));
	return temp;
}

struct ast* nud(struct token* token){
	struct ast* ret_val;
	if(token->t == i_ltrl || token->t == str_ltrl)
		ret_val= make_term_tree(token);

	else if(token->t == identifier){
		if (strcmp (peek()->str, "(") != 0)
			ret_val= make_term_tree(token);			
		else{			//do function call stuff
			next();//eat up the '('
			struct ast* temp = make_comma_tree();

			expected(")", next()->str);//eats up the )

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
		struct ast *temp = expr(0);
		struct token *delimiter = next();

		expected(")", delimiter->str);
		ret_val=temp;
		//maybe later add a field to the `(` that contains pointer to `)`
	}
	else{
		fprintf(stderr,"expected expression before: '%s'", token->str);
		exit(1);
	}
	return ret_val;
}

struct ast* expr(int rbp){
    struct ast* left = nud(next());
    while (bp(peek(),infix) > rbp)
        left = led(left, next());
    return left;
}

static inline void print_indent(int i){
	for (;i>0;i--)
		printf("|   ");
	printf("-> ");
}

void print_ast(struct ast* root, int indent){
	print_indent(indent);
	switch (root->type){
		case term:
			print_tokens(root->token, 1);
			break;

		case ternary_op:
			print_tokens(root->op, 1);
			for(int i=0;i<3;i++)
				print_ast(root->term[i], indent+1);
			break;

		case bin_op:
			print_tokens(root->op, 1);
			for(int i=0;i<2;i++)
				print_ast(root->term[i], indent+1);
			break;

		case unary_op:
			print_tokens(root->op, 1);
			print_ast(root->term[0], indent+1);
			break;

		case func_call:
			print_tokens(root->func_name, 1);
			for (int i=0; i<root->argc; i++){
				print_ast(root->argv[i],indent+1);
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

void free_ast(struct ast* root){
	switch (root->type){
		case term:
			break;
		case ternary_op:
			free_ast(root->term[2]);	//fallthrough
		case bin_op:
			free_ast(root->term[1]);	//fallthrough
		case unary_op:
			free_ast(root->term[0]);
			break;
		case func_call:
		case comma:
			for (int i=0; i<root->argc;i++){
				free_ast(root->argv[i]);
			}
			free(root->argv);
			break;
		case unprocessed:
			fprintf(stderr,"encountered an unprocessed tree while freeing");
			break;		
	}
	free(root);
}
