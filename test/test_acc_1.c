#include "decl_parser.h"
#include "lexer.h"

#include <inttypes.h>
#include <stdio.h>

int main(void) {
    const char cdecls[][200] = {
        "const const const int",
        "register auto char int",
        "signed float x",
        "double x",
    };
    uint32_t expected_type[] = {
        SPEC_CONST | SPEC_INT,
        SPEC_REGISTER,
        SPEC_SIGNED,
        SPEC_DOUBLE,
    };
    int expected_status[] = {
        0,
        2,
        2,
        1,
    };
    struct lex_context ctx;

    for (unsigned i = 0; i < sizeof cdecls / sizeof *cdecls; i++) {
        lex_inits(&ctx, cdecls[i]);
        int id;
        int acc_status = 0;
        uint32_t current_type = 0;
        for (id = lex_next(&ctx); acc_status == 0 && id != LEX_TKEOI;
             id = lex_next(&ctx))
            acc_status = accumulate(id, &current_type);
        if (acc_status != expected_status[i]) {
            fprintf(stderr, "for input '%s':\n", cdecls[i]);
            fprintf(stderr, "expected return value %d, got %d\n",
                    expected_status[i], acc_status);
            return 1;
        }
        if (acc_status != expected_status[i]) {
            fprintf(stderr, "for input '%s':\n", cdecls[i]);
            fprintf(stderr, "expected type to be %d, got %d\n",
                    expected_type[i], current_type);
            return 2;
        }
    }
    return 0;
}
