#ifndef DECL_PARSER_H
#define DECL_PARSER_H
/*	this file parses a declaration and adds it
	to the symbol table if needed	*/
#include "type.h"
#include "symbol_table.h"
#include "pratt.h"

struct decl_ast{
	struct symbol s;
	//a pointer to the symbol table
	struct expr_ast* init_val;
	//the initial value, if exists. otherwise a NULL
};

enum type_specs_mask{//
	SPEC_NONE=0,
	SPEC_VOID=1<<9,
	SPEC_CHAR=2<<9,
	SPEC_INT=3<<9,
	SPEC_FLOAT=4<<9,
	SPEC_DOUBLE=5<<9,
	SPEC_CORE_TYPE=0x1F<<9,
	SPEC_IDENTIFIER_TYPE=0x1F<<9,
	//0-4 core typ<<9e
	SPEC_SIGNED = 3<<14,
	SPEC_UNSIGNED = 1<<14,
	SPEC_HAS_SIGN = 1<<14,
	//5=has sign, 6-signed/unsigne<<9d
	SPEC_SHORT = 1<<16,
	SPEC_LONG = 1<<17,
	SPEC_LONG_LONG = 1<<18,
	//bits 7-9 - length modifiers
};
enum storage_specs_mask{
	SPEC_TYPEDEF = 1<<5	,
	SPEC_EXTERN  = 2<<5	,
	SPEC_STATIC  = 3<<5	,
	SPEC_AUTO    = 4<<5	,
	SPEC_REGISTER= 5<<5	,
};
enum type_quals_mask{
    SPEC_CONST    = 1,
    SPEC_RESTRICT = 2,
    SPEC_VOLATILE = 4,
    SPEC_INLINE   = 8,
};

int accumulate(int t_id,uint32_t* specs);

void full_decl(void);
//parse a full declaration

#endif
