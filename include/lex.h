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

typedef enum {
    L_SUCCESS = 0,
    L_NO_SUCH_FILE,
    L_OUT_OF_MEMORY,
    L_UNREADABLE_FILE,
    L_ERR_NO_INPUT,
    L_UNEXPECTED_EOF,
    L_BAD_ESCAPE_CHAR,
    L_INVALID_UTF8,
    L_UNTERMINATED_STRING
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

Token * token_alloc(Lexer * lexer);
void token_free(Token * token);

Lexer * lexer_alloc(const char * file_name, const char * input);
void lexer_free(Lexer * lexer);

Token * lex(Lexer * lexer, LexerStatus * status);

const char * lex_identifier(Lexer * lexer);

struct token_loc get_loc(Lexer * lexer);

int take_ws(Lexer * lexer);

wchar_t get_escaped_char(wchar_t c, LexerStatus * status);

