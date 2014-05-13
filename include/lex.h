// vim: ft=c

typedef int (CharTestFn)(int c);

enum lexer_state {L_START, L_STRING, L_END};

typedef struct {
    const char * file_name;
    const char * input;
    int _p;
    int cur_line;
    int cur_column;
    enum lexer_state state;
} Lexer;

/**
 * All these values should be negative with the exception of L_SUCCESS
 */
typedef enum {
    L_SUCCESS = 0,
    L_ERR_NO_INPUT=-1,
    L_UNEXPECTED_EOF=-2,
    L_BAD_ESCAPE_CHAR=-3,
    L_INVALID_UTF8=-4,
    L_UNTERMINATED_STRING=-5
} LexerStatus;

enum token_type {T_IDENT, T_STRING, T_EOF};

struct token_loc {
    int line;
    int column;
};

typedef struct {
    enum token_type type;
    Lexer * lexer;
    const char * lexeme;
    struct token_loc start_loc;
    struct token_loc end_loc;
} Token;

Token * token_new(Lexer * lexer);

Lexer * lexer_new(const char * file_name, const char * input);

Token * lex(Lexer * lexer, LexerStatus * status);

const char * lex_identifier(Lexer * lexer);

struct token_loc get_loc(Lexer * lexer);

