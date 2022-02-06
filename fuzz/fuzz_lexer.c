#include "lexer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 1024

int main(void) {
    // Yes, I copied this off stackoverflow
    char buffer[BUF_SIZE];
    size_t contentSize = 1;
    char *content = malloc(sizeof(char) * BUF_SIZE);
    if (content == NULL) {
        perror("Failed to allocate content");
        exit(1);
    }

    content[0] = '\0';
    while (fgets(buffer, BUF_SIZE, stdin)) {
        char *old = content;
        contentSize += strlen(buffer);
        content = realloc(content, contentSize);
        if (content == NULL) {
            perror("Failed to reallocate content");
            free(old);
            exit(2);
        }
        strcat(content, buffer);
    }

    if (ferror(stdin)) {
        free(content);
        perror("Error reading from stdin.");
        exit(3);
    }

    struct lex_context ctx;
    int id;
    lex_inits(&ctx, content);
    do id = lex_next(&ctx);
    while (id != LEX_TKEOI);
    return 0;
}
