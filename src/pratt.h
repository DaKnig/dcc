#ifndef PRATT_H
#define PRATT_H
/*******************************************************************************
	This library implements an expression parser for C expressions using the
	Pratt parsing algorithm.
	To get a full expression, call full_expression()
	It advances the token_stream only when a token is consumed, meaning that it
	does not check or consume the delimiter of the expression
*******************************************************************************/

enum ast_type   {term, unary_op, bin_op, ternary_op, comma, func_call,
																unprocessed};

struct token* token_stream_head;
struct token* token_stream;
unsigned token_stream_len;


struct expr_ast{
	enum ast_type type;
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

struct expr_ast* expr(int bp);
struct expr_ast* full_expression(void);
void print_expr_ast(struct expr_ast* root, int indent);
void free_expr_ast(struct expr_ast* root);

#endif
