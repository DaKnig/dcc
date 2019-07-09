/*
    the format of types (saved as uint32_t):
    a few bits in each category are reserved for future expansion
    bit 0-4 - type qualifiers:
        const=1, restrict=2, volatile=4, inline=8
    bit 5-8 - storage class specifiers
        typedef,extern,static,auto,register - according to the SPEC masks
    bit 9-? - type specifier, seperated into "sub fields"
        bit 9-13 - "core type"- one of:{void,char,int,float,double,none,typedefed}
        bit 14-15- "sign information" - signed, unsigned or neither
        bit 16-18- "length modifier" - short, long, long long
 */

/*
    built in types:
    —void
    —char
    —signed char
    —unsigned char
    —short,signed short,short int, or signed short int
    —unsigned short, or unsigned short int
    —int,signed, or signed int
    —unsigned, or unsigned int
    —long,signed long,long int, or signed long int
    —unsigned long, or unsigned long int
    —long long,signed long long,long long int, or signed long long int
    —unsigned long long, or unsigned long long int
    —float
    —double
    —long double
    —_Bool
    —float_Complex
    —double_Complex
    —long double_Complex
    —  atomic type specifier
    —  struct or union specifier
    —  enum specifier
    —  typedef name
*/


#include "symbol_table.h"
#include "tokenizer.h"
#include "pratt.h"
#include "type.h"
#include "lexer.h"
#include "token.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define lex_id(s) lex_tk_keyword(&useless_token, s, 0, 0)
struct lex_token useless_token;


struct decl_ast{
    unsigned s;
    //an index in the symbol table
    struct expr_ast* init_val;
    //the initial value, if exists. otherwise a NULL
};

struct decl_ast* decls;
unsigned max_decl_count;
unsigned decl_count;

static inline uint32_t storage_class_specs(uint32_t specs){
    return specs>>5 & 0x0F;//4 bits starting at bit 5
}
static inline uint32_t type_specs(uint32_t specs){
    return specs>>9 & 0x1FF;//9 bits starting at bit 9
    //see table below
}
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

int accumulate(int t_id,uint32_t* specs){
//	current type saved in specs. new token in t.
//	returns 0 if accumulated successfully into specs
//	returns 1 if encountered an identifier
//	returns 2 for conflicting specifiers
//	returns 3 for other errors
    if (t_id==LEX_TKIDENTIFIER)
        return 1;
    uint32_t storage=storage_class_specs(*specs);
    uint32_t core=type_specs(*specs)&SPEC_CORE_TYPE;
    uint32_t condition=0;
    uint32_t sign;
    uint32_t mask;
    switch(t_id){	//if type qualifiers:
    //bits 0-4 are the type qualifiers
        case LEX_TKCONST:	condition=0; mask=SPEC_CONST;    break;
        case LEX_TKRESTRICT:condition=0; mask=SPEC_RESTRICT; break;
        case LEX_TKVOLATILE:condition=0; mask=SPEC_VOLATILE; break;
        case LEX_TKINLINE:	condition=0; mask=SPEC_INLINE;   break;
    //bits 5-8 are storage class specifiers
    //accumulate storage class specifier if none was specified already
        case LEX_TKTYPEDEF: condition=storage; mask=SPEC_TYPEDEF;	break;
        case LEX_TKEXTERN:	condition=storage; mask=SPEC_EXTERN; 	break;
        case LEX_TKSTATIC:	condition=storage; mask=SPEC_STATIC;	break;
        case LEX_TKAUTO:	condition=storage; mask=SPEC_AUTO;		break;
        case LEX_TKREGISTER:condition=storage; mask=SPEC_REGISTER;	break;
    //if type specifier:
    //bits 9-14 bits are type specifier
        case LEX_TKVOID:	condition=core; mask=SPEC_VOID;	break;
        case LEX_TKINT:		condition=core; mask=SPEC_INT;	break;
        case LEX_TKCHAR:	condition=core; mask=SPEC_CHAR;	break;
        case LEX_TKFLOAT:	condition=core; mask=SPEC_FLOAT;break;
        case LEX_TKDOUBLE:	condition=core; mask=SPEC_DOUBLE;break;
        case LEX_TKSIGNED:
            mask=SPEC_SIGNED;
accumulate_sign:
            switch(type_specs(*specs)&(SPEC_CORE_TYPE|SPEC_SIGNED)){
                case SPEC_NONE://fallthrough
                case SPEC_CHAR://fallthrough
                case SPEC_INT:
                    condition=1;	break;
            }
            break;
        case LEX_TKUNSIGNED:
            mask=SPEC_UNSIGNED;
            goto accumulate_sign;
        case LEX_TKSHORT:
            switch(type_specs(*specs)&(~SPEC_SIGNED)){
                //sign does not matter
                case SPEC_NONE:		//fallthrough
                case SPEC_INT:
                    mask=SPEC_SHORT;
                    condition=1;	break;
            }
            break;
        case LEX_TKLONG:
            switch(type_specs(*specs)&(~SPEC_SIGNED)){
                case SPEC_LONG:				//fallthrough
                case SPEC_LONG|SPEC_INT:	//fallthrough
                    *specs^=(SPEC_LONG|SPEC_LONG_LONG);
                        //turn off the LONG bit, turn on the LONG_LONG bit
                    return 0;
                case SPEC_NONE:		//fallthrough
                case SPEC_INT:		//fallthrough
                case SPEC_DOUBLE:
                    mask=SPEC_LONG;
                    condition=1;	break;
                default: break;
            }
            break;
        default: break;
    }
    if(condition)
        return 2;
    else{
        *specs|=mask;
        return 0;
    }
}
/*
void full_decl(void){
    //parses a full declaration
    //only call this when lex_next() would return a type or a modifier
    struct type decl_type;
    decls = NULL;
    max_decl_count=4;
    decl_count=0;


    if (decls=malloc(max_decl_count))
        exit(fprintf(stderr,"malloc: decls"));


    int temp=lex_next();
    decl_type.general_type=var;
    // if (lex_id("int")==temp)
    // 	decl_type.p=type_int;
    // if (lex("char",temp.str)==0)	decl_type.p=type_char;
    decl_type.id=lex_id(temp);

    struct decl_ast temp;

    while(is_type(lex_peek())){
        {}//TODO- allow per-variable specifiers
        struct symbol s = {.t=decl_type , .name=lex_next().str , .access=true};
        temp.s = add_to_table(s);
        if(lex_peek().str[0] == ','){
            lex_getnext();
        }
    }
}
*/
