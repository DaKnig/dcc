#ifndef PRATT_H
#define PRATT_H
/*******************************************************************************
	This library implements an expression parser for C expressions using the
	Pratt parsing algorithm.
	To get a full expression, call full_expression()
	It advances the token_stream only when a token is consumed, meaning that it
	does not check or consume the delimiter of the expression
*******************************************************************************/
struct context;

enum expr_ast_type
{
	term,
	unary_op,
	bin_op,
	ternary_op,
	comma,
	func_call,
	unprocessed
};


struct expr_ast{
	enum expr_ast_type type;
	union{
		struct{//term
			struct token* token;
		};
		struct{//op - unary, binary, ternary
			struct token* op;
			struct expr_ast* term[3];
		};
		struct{//func call,comma///fix is required!!!
			struct token* func_name;
			struct expr_ast* *argv;
			int argc;
			//pointer to a block
		};
	};
};

enum notation{prefix, infix};
int bp(struct context* input,
       const struct token* token, const enum notation n);
/*
  returns a binding power suitable for use in expr()
*/

struct expr_ast* expr(int bp, struct context* input);
// struct expr_ast* full_expression(void);
void print_expr_ast(struct expr_ast* root, int indent);
void free_expr_ast(struct expr_ast* root);

#endif
