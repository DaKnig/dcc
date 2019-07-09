#include "lexer.h"
int main(void){
    struct lex_context ctx;
    int id;
    lex_inits(&ctx,"signed float x");
    do
        id=lex_getnext(&ctx);
    while(id!=LEX_TKEOI);
    return 0;
}
