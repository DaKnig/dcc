#ifndef TOKENIZER_H
#define TOKENIZER_H

enum token_type{
		op, i_ltrl, str_ltrl, identifier,
};
struct token{
	char* str;
	enum token_type t;
};

unsigned tokenize(struct token** tokens, char* str, const char** ops, int ops_n);

void print_tokens(struct token* tokens, unsigned n);

void free_tokens(struct token* token_stream_head, unsigned token_stream_len);
#endif
