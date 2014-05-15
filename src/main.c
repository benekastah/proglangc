
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include "lex.h"

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");
    Token * tok;
    Lexer * lexer;

    char * fname = argv[1];
    FILE * fin;
    long lsize;
    char * buffer;

    // Read file into memory
    fin = fopen(fname, "rb");
    if (!fin) {
        perror("Error");
        return L_NO_SUCH_FILE;
    }
    fseek(fin, 0L, SEEK_END);
    lsize = ftell(fin);
    rewind(fin);
    buffer = malloc(lsize + 1);
    if (!buffer) {
        fclose(fin);
        fputs("Could not allocate memory", stderr);
        return L_OUT_OF_MEMORY;
    }
    if (!fread(buffer, lsize, 1, fin)) {
        fclose(fin);
        free(buffer);
        fputs("Failed to read entire file", stderr);
        return L_UNREADABLE_FILE;
    }
    fclose(fin);

    lexer = lexer_alloc("", buffer);

    LexerStatus lexer_status = L_SUCCESS;
    while (lexer->state != L_END) {
        tok = lex(lexer, &lexer_status);
        switch (lexer_status) {
            case L_SUCCESS:
                break;
            case L_ERR_NO_INPUT:
                printf("Error: no input was supplied to the lexer.\n");
                break;
            case L_UNEXPECTED_EOF:
                printf("Error: unexpected end of input.\n");
                break;
            case L_INVALID_UTF8:
                printf("Error: invalid utf8.\n");
                break;
            default:
                printf("The lexer was unable to lex.\n");
        }
        if (lexer_status != L_SUCCESS) {
            token_free(tok);
            break;
        }
        printf("{ type = %i, lexeme = \"%s\" } at (%i,%i) - (%i,%i)\n",
                tok->type,
                tok->lexeme,
                tok->start_loc.line,
                tok->start_loc.column,
                tok->end_loc.line,
                tok->end_loc.column);
        token_free(tok);
    }
    free(buffer);
    lexer_free(lexer);
    return lexer_status;
}
