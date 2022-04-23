#include "top_level_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//those are globals- because the whole program uses them
//extern struct token* token_stream_head;
//extern struct token* token_stream;
//extern unsigned token_stream_len;

int main(int argc, char** argv) {
    FILE* in_file = NULL;

    switch (argc) {
    case 1:
        in_file = stdin;
        argv[1] = "<stdin>";
        break;
    case 2: in_file = fopen(argv[1], "r"); break;
    default: printf("too many arguments"); return 1;
    }

    if (!in_file) {
        perror("input file error: ");
        return 1;
    }

    struct context* input = create_ctx(in_file);
    struct init_declaration_list* s = parse_translation_unit(input);
    printf("tree representation of %s:", argv[1]);
    print_declaration(s, 0);
    return 0;
}
