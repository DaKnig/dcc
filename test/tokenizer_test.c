#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DEBUG
#include "new_tokenizer.h"
#undef DEBUG

int main(void) {
    struct {
        char* input_declaration; // the declaration as a string
        size_t token_count; // how many tokens there are in a test?
        const char* first_token; // last token as a string
        const char* last_token;
    } expected_results[]
        = {{"int* foo;", 4, "int", ";"}, {"int a,b;", 5, "int", ";"}};
    for (unsigned i = 0;
         i < (sizeof(expected_results) / sizeof(*expected_results)); i++) {
        char* declaration = expected_results[i].input_declaration;
        struct context* input
            = create_ctx(fmemopen(declaration, strlen(declaration), "r"));
        // turns the string into a read-only file; makes a context out of it
        printf("input string: `%s`\n", declaration);

        struct token t;
        expect_token(expected_results[i].first_token, input);

        size_t count;
        for (t = *peek(input), count = 1; peek(input)->t != t_EOF; count++) {
            struct token* peek_token = peek(input);
            struct token* next_token = next(input);
            assert(peek_token == next_token);
            // check the sanity in the relation between peek, next.

            t = *next_token;
            print_token(&t);
        }

        if (count != expected_results[i].token_count) {
            fprintf(stderr, "expected %zd tokens, got %zd\n",
                    expected_results[i].token_count, count);
            exit(1);
        }

        if (0 != strcmp(t.str, expected_results[i].last_token)) {
            fprintf(stderr, "expected %s before %s\n",
                    expected_results[i].last_token, t.str);
            exit(1);
        }

        fclose(input->file); // close the string file
    }
    return 0;
}
