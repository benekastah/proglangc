
#include <locale.h>
#include <stdio.h>

#include "lex.h"

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");
    Token * tok;
    Lexer * lexer = lexer_alloc("", argv[1]);
    LexerStatus lexer_status;
    while (lexer->state != L_END) {
        tok = lex(lexer, &lexer_status);
        printf("{ type = %i, lexeme = \"%s\" } at (%i,%i) - (%i,%i)\n",
                tok->type,
                tok->lexeme,
                tok->start_loc.line,
                tok->start_loc.column,
                tok->end_loc.line,
                tok->end_loc.column);
        token_free(tok);
    }
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

    lexer_free(lexer);
    return lexer_status;
}
