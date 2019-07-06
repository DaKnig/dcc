/*
	this file describes the structure of a statement_ast
	for now, should not be included anywere
 */
#ifndef STATEMENT_AST_H
#define STATEMENT_AST_H
#include "pratt.h"
#include "decl_parser.h"

struct statement_ast{
	enum {expression,declaration,} type;
	union{
		struct expr_ast* expr;
		struct decl_ast* decl;
	};
		
};
#endif
