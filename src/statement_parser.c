#include "statement_parser.h"
#include "pratt.h"
#include "new_tokenizer.h"
#include "util.h"
#include "decl_parser.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline int is_type(char* str){
    char types[][8] = {
        "void", "char", "int", "float", "double",
        "signed", "unsigned", "long", "short",
        "typedef", "extern", "static", "auto",
        "register", "const", "restrict", "volatile",
        "inline"
    };
    for (unsigned i=0; i< sizeof types / 8 ; i++)
        if (strcmp(str,types[i]) == 0)
            return 1;
    return 0;
}

static inline void parse_expression_then_statement(struct statement* s,
			struct context* input){
    //parses if, switch, while
    next(input);
    expect_token("(",input);
    s->e=expr(0,input);
	expect_token(")",input);
    s->s=parse_statement(input);
}

struct statement* parse_statement(struct context* input){
//	'''returns the statement it parses'''
	const struct token* t=peek(input);

    struct statement* s=xmalloc(sizeof (struct statement));

	if (strcmp(t->str, "if") == 0) {
		parse_expression_then_statement(s,input);
		if (strcmp(peek(input)->str,"else"))
			s->t=s_if;
		else {
			next(input);
			s->if_false=parse_statement(input);
			s->t=s_if_else;
		}
		return s;
    }
	if (strcmp(t->str, "switch")==0) {
		parse_expression_then_statement(s,input);
		s->t=s_switch;
		return s;
	}
	else if (strcmp(t->str,"while")==0) {
		parse_expression_then_statement(s,input);
		s->t=s_while;
		return s;
	}
	else if (strcmp(t->str, "do")==0) {
		next(input);
		s->s=parse_statement(input);
		expect_token("while",input);
		expect_token("(",input);
		s->e=expr(0,input);
		expect_token(")",input);
		expect_token(";",input);
		return s;
	}
	else if(strcmp(t->str,"for")==0) {
		next(input);
		expect_token("(",input);
		s->expr[0]=expr(0,input);
		expect_token(";",input);
		s->expr[1]=expr(0,input);
		expect_token(";",input);
		s->expr[2]=expr(0,input);
		expect_token(")",input);
		s->s=parse_statement(input);
		return s;
	}
	else if(strcmp(t->str,"{")==0) {
		next(input);
		s->t=s_compound;
		s->block.size=1, s->block.e=NULL;
		while (strcmp(peek(input)->str, "}")!=0) {/*add proper EOF checking here*////
			if(((s->block.size-1) & s->block.size) == 0)
				s->block.e=xrealloc(s->block.e,
					sizeof(struct block_element)*s->block.size);

			struct block_element* last = &s->block.e[s->block.size-1];
			//the newly allocated block element
			{
				if (is_type(peek(input)->str)) {
					//if it is a type-related keyword, parse declaration
					last->t=s_declaration;
					last->d=parse_declaration(input);
				}
				else {
					last->t=s_statement;
					//last->s=xmalloc(sizeof (struct statement));
					last->s=parse_statement(input);
				}
			}
		}
		expect_token("}",input);
	}
	else if (strcmp(t->str,"case")==0){
		next(input);
		s->t=s_case;
		expect_token(":",input);
		s->s=parse_statement(input);
	}
	else if (t->str[0] == ';') {
		next(input);
		s->t=s_expression;
		s->e=NULL;
	}
	else{//	#then we're probably parsing an expression
		s->t=s_expression;
		s->e=expr(0,input);
		expect_token(";",input);
	}
	return s;
}

void print_statement(struct statement* s, int indent){
	for (;indent!=0;indent--)	printf("    ");
	switch(s->t){
		case s_case:
			puts("case");
			print_expr_ast(s->e,indent+1);
			for (indent++;indent!=0;indent--)	printf("    ");
			puts(":");
			print_statement(s->s,indent);
			break;
		case s_label:
			printf("%s:\n",s->label);
			print_statement(s->s,indent);
			break;
		case s_default:
			printf("default:\n");
			print_statement(s->s,indent);
			break;
		case s_compound:;
			struct block* b=&s->block;
			for(unsigned i=0; i<b->size; i++){
				if (b->e[i].t==s_statement)
					print_statement(b->e[i].s,indent+1);
				else
					print_declaration(b->e[i].d,indent+1);
			}
			break;
		case s_expression:
			if (s->e!=NULL)
				print_expr_ast(s->e,indent);
			else
				puts(";");
			break;
		case s_if:
			puts("if");
			print_expr_ast(s->e,indent+1);
			puts("");
			print_statement(s->s, indent+1);
			break;
		case s_if_else:
			puts("if");
			print_expr_ast(s->e,indent+1);
			puts("");
			print_statement(s->if_true, indent+1);
			for (;indent!=0;indent--)	printf("    ");
			puts("else");
			print_statement(s->if_true, indent+1);
			break;
		case s_switch:
			puts("switch");
			print_expr_ast(s->e,indent+1);
			puts("");
			print_statement(s->s, indent+1);
			break;
		case s_for:
			puts("for");
			for (unsigned i=0;i<3;i++) {
				for (;indent!=0;indent--)	printf("    ");
				printf("expr%d:\n",i);
				print_expr_ast(s->expr[i],indent+1);
			}
			break;
		case s_while:
			puts("while");
			print_expr_ast(s->e,indent+1);
			puts("");
			print_statement(s->s, indent+1);
			break;
		case s_do_while:
			puts("do");
			print_statement(s->s, indent+1);
			for (;indent!=0;indent--)	printf("    ");
			puts("..while");
			print_expr_ast(s->e,indent+1);
			break;
		case s_goto:
			printf("goto %s\n",s->label);
			break;
		case s_continue:
			puts("continue");
			break;
		case s_break:
			puts("break");
			break;
		case s_return:
			puts("return");
			if (s->e!=NULL)
				print_expr_ast(s->e,indent+1);
			break;
		default:
			printf("something really bad has happend!");
			exit(1);
	}
}

void print_block_element(struct block_element* element,int indent){
	if (element->t == s_declaration)
		print_declaration(element->d,indent);
	else if (element->t == s_statement)
		print_statement(element->s,indent);
	else
		printf("error");
}
