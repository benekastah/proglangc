
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lex.h"

/*
 * Really helpful docs:
 * http://www.gnu.org/software/libc/manual/html_node/Classification-of-Characters.html
 */

Lexer * lexer_new(const char * file_name, const char * input) {
    Lexer * lexer = malloc(sizeof(Lexer));
    lexer->file_name = file_name;
    lexer->input = input;
    lexer->_p = 0;
    lexer->cur_line = 1;
    lexer->cur_column = 1;
    lexer->state = L_START;
    return lexer;
}

Token * token_new(Lexer * lexer) {
    Token * token = malloc(sizeof(Token));
    token->lexer = lexer;
    token->lexeme = 0;
    return token;
}

char nextchar(Lexer * lexer) {
    return lexer->input[lexer->_p++];
}

char * substr(Lexer * lexer, int start, int end) {
    int size = end - start;
    char * str = (char *)malloc(size + 1);  // str needs a null byte at the end
    memcpy(str, lexer->input + start, size);
    str[size] = 0;
    return str;
}

char * lastn_substr(Lexer * lexer, int len) {
    return substr(lexer, lexer->_p - len, lexer->_p);
}

int isdblquote(int c) {
    if (c == '"') {
        return 1;
    } else {
        return 0;
    }
}

int isescapemeta(int c) {
    if (c == '\\') {
        return 1;
    } else {
        return 0;
    }
}

int take_n(Lexer * lexer, int n, CharTestFn * test) {
    int len = 0;
    char curchar;
    while ((n <= 0 || len < n)) {
        curchar = nextchar(lexer);
        if ((*test)(curchar)) {
            len += 1;
            if (curchar == '\n') {
                lexer->cur_line += 1;
                lexer->cur_column = 1;
            } else if (curchar != '\r') {
                lexer->cur_column += 1;
            }
        } else {
            lexer->_p -= 1;
            break;
        }
    }
    return len;
}

int take(Lexer * lexer, CharTestFn * test) {
    return take_n(lexer, 0, test);
}

int take_one(Lexer * lexer, CharTestFn * test) {
    return take_n(lexer, 1, test);
}

int take_ws(Lexer * lexer) {
    return take(lexer, &isspace);
}

int take_identifier(Lexer * lexer) {
    int len = 0;
    if (len += take_one(lexer, &isalpha)) {
        len += take(lexer, &isalnum);
    }
    return len;
}

int take_string_simple(Lexer * lexer) {
    int len = 0;
    if (take_one(lexer, &isdblquote)) {
        len += 1;
        while (!take_one(lexer, &isdblquote)) {
            len += 1;
        }
        len += 1;
    }
    return len;
}

char get_escaped_char(char c, LexerStatus * status) {
    switch (c) {
        case 'b': return '\b';
        case 't': return '\t';
        case 'n': return '\n';
        case 'v': return '\v';
        case 'f': return '\f';
        case 'r': return '\r';
        case '"': return '"';
        case '\\': return '\\';
        default: *status = L_BAD_ESCAPE_CHAR;
    }
    return 0;
}

/**
 * Important note: string lexers should have the string in question as the only
 * input. parse_string, therefore, can build a string knowing that the input
 * starts and ends with ".
 */
const char * parse_string(Lexer * lexer, LexerStatus * status) {
    size_t len = 1;  // One for the terminating null byte
    int i;
    char c;
    for (i = 0; (c = lexer->input[i]) != '\0'; i++) {
        if (!(isdblquote(c) || isescapemeta(c))) {
            len += 1;
        }
    }
    char * str = malloc(len);
    int p = 0;
    int escaping = 0;
    for (i = 0; (c = lexer->input[i]) != '\0'; i++) {
        if (escaping) {
            escaping = 0;
            str[p++] = get_escaped_char(c, status);
            if (*status == L_BAD_ESCAPE_CHAR) {
                goto err;
            }
        } else if (isescapemeta(c)) {
            escaping = 1;
        } else if (isdblquote(c)) {
            continue;
        } else {
            str[p++] = c;
        }
    }
    str[p] = 0;
    return str;

err:
    return str;
}

struct token_loc get_loc(Lexer * lexer) {
    struct token_loc loc = {lexer->cur_line, lexer->cur_column};
    return loc;
}

void token_merge(Token * tok1, Token * tok2) {
    tok1->lexeme = tok2->lexeme;
    tok1->type = tok2->type;
}

Token * lex(Lexer * lexer, LexerStatus * status) {
    if (lexer->input == NULL) {
        *status = L_ERR_NO_INPUT;
        goto err;
    }

    Token * tok = token_new(lexer);
    tok->start_loc = get_loc(lexer);
    int len = 0;
    Lexer *string_lexer;

    switch (lexer->state) {
        case L_START:
            if ((len = take_identifier(lexer))) {
                tok->type = T_IDENT;
            } else if (take_string_simple(lexer)) {
                // After getting the whole string, make a new lexer to parse
                // it. This is so we don't have to guess at how big the string
                // is.
                string_lexer = lexer_new(
                    lexer->file_name, lastn_substr(lexer, len));
                string_lexer->state = L_STRING;
                token_merge(tok, lex(lexer, status));
            } else {
                lexer->state = L_END;
                token_merge(tok, lex(lexer, status));
            }
            break;
        case L_STRING:
            tok->type = T_STRING;
            tok->lexeme = parse_string(lexer, status);
            if (*status != L_SUCCESS) {
                goto err;
            }
            break;
        case L_END:
            tok->type = T_EOF;
            break;
    }
    if (!tok->lexeme) {
        tok->lexeme = lastn_substr(lexer, len);
    }
    if (!tok->end_loc.line || !tok->end_loc.column) {
        tok->end_loc = get_loc(lexer);
    }
    take_ws(lexer);
    return tok;

err:
    lexer->state = L_END;
    return tok;
}

int main(int argc, char* argv[]) {
    Token * tok;
    Lexer * lexer = lexer_new("", argv[1]);
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
    }
    if (lexer_status == L_ERR_NO_INPUT) {
        printf("Error: no input was supplied to the lexer.\n");
    } else if (lexer_status != L_SUCCESS) {
        printf("The lexer was unable to lex.\n");
    }
    return lexer_status;
}
