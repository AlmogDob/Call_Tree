/**
 * @file
 * @brief A small single-header lexer for C/C++-like source text.
 *
 * The lexer operates on a caller-provided, read-only character buffer.
 * It produces tokens that reference slices of the original buffer (no
 * allocations and no null-termination guarantees).
 *
 * @note This header depends on "Almog_String_Manipulation.h" for the
 *       `asm_*` character classification and string helper routines used
 *       by the implementation (e.g. `asm_isalpha`, `asm_isspace`, etc.).
 * @note This single header library is insapired by
 * Tsoding's C-lexer implementation: https://youtu.be/AqyZztKlSGQ
 */

#ifndef ALMOG_LEXER_H_
#define ALMOG_LEXER_H_

#include "Almog_String_Manipulation.h"
#include "Almog_Dynamic_Array.h"

/**
 * @def AL_ASSERT
 * @brief Assertion macro used by the lexer (defaults to @c assert()).
 *
 * Define @c AL_ASSERT before including this header to override.
 */
#ifndef AL_ASSERT
#include <assert.h>
#define AL_ASSERT assert
#endif /* AL_ASSERT */ 

/**
 * @def AL_MALLOC
 * @brief Deallocation macro used by al_lexer_alloc() (defaults to @c malloc()).
 *
 * Define @c AL_MALLOC before including this header to override.
 */
#ifndef AL_MALLOC
#include <stdlib.h>
#define AL_MALLOC malloc
#endif /* AL_MALLOC */ 

/**
 * @def AL_FREE
 * @brief Deallocation macro used by al_tokens_free() (defaults to @c free()).
 *
 * Define @c AL_FREE before including this header to override.
 */
#ifndef AL_FREE
#include <stdlib.h>
#define AL_FREE free
#endif /* AL_FREE */ 

/**
 * @enum Token_Kind
 * @brief Token categories produced by the lexer.
 *
 * The lexer attempts to classify source text into:
 * - high-level "word-like" tokens (identifiers, keywords, literals, comments)
 * - punctuation / operators (matched using the longest-match rule)
 * - TOKEN_INVALID for unrecognized or malformed sequences
 * - TOKEN_EOF at end of input
 */
enum Token_Kind {
    /* Sentinel / unknown */
    TOKEN_EOF,
    TOKEN_INVALID,

    /* High-level / multi-char / "word-like" */
    TOKEN_PP_DIRECTIVE,
    TOKEN_COMMENT,
    TOKEN_STRING_LIT,
    TOKEN_CHAR_LIT,
    TOKEN_INT_LIT_BIN,
    TOKEN_INT_LIT_OCT,
    TOKEN_INT_LIT_DEC,
    TOKEN_INT_LIT_HEX,
    TOKEN_FLOAT_LIT_DEC,
    TOKEN_FLOAT_LIT_HEX,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,


    /* Grouping / separators */
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    /* Punctuation */
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_BSLASH,
    TOKEN_HASH,

    /* Ternary */
    TOKEN_QUESTION,
    TOKEN_COLON,

    /* Assignment / equality */
    TOKEN_EQ,
    TOKEN_EQEQ,
    TOKEN_NE,
    TOKEN_BANG,

    /* Relational */
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,

    /* Bitwise / boolean */
    TOKEN_BITAND,
    TOKEN_ANDAND,
    TOKEN_BITOR,
    TOKEN_OROR,
    /* Bitwise unary */
    TOKEN_CARET,
    TOKEN_TILDE,

    /* Shifts */
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,

    /* Inc / dec */
    TOKEN_PLUSPLUS,
    TOKEN_MINUSMINUS,

    /* Arithmetic */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    /* Compound assignment */
    TOKEN_PLUSEQ,
    TOKEN_MINUSEQ,
    TOKEN_STAREQ,
    TOKEN_SLASHEQ,
    TOKEN_PERCENTEQ,
    TOKEN_ANDEQ,
    TOKEN_OREQ,
    TOKEN_XOREQ,
    TOKEN_LSHIFTEQ,
    TOKEN_RSHIFTEQ,

    /* Member access / varargs */
    TOKEN_ARROW,
    TOKEN_ELLIPSIS,
};

/**
 * @struct Literal_Token
 * @brief Mapping between a literal operator/punctuation text and a token kind.
 *
 * Used internally for longest-match scanning of operators and punctuation.
 *
 * @note `text` must be a null-terminated string literal.
 */
struct Literal_Token {
    enum Token_Kind kind;
    const char * const text;
};

/**
 * @struct Location
 * @brief Source location (1-based externally in produced tokens).
 *
 * `al_lexer_next_token()` stores:
 * - line_num: 1-based line number
 * - col: 1-based column number
 */
struct Location {
    size_t line_num;
    size_t col;
};

/**
 * @struct String
 * @brief Simple dynamic array of characters (used to hold file content).
 *
 * This struct is compatible with the dynamic array macros from
 * "Almog_Dynamic_Array.h".
 */
struct String {
    size_t length;
    size_t capacity;
    char* elements;
};

/**
 * @brief A token produced by the lexer.
 *
 * `text` points into the original input buffer passed to @ref al_lexer_alloc.
 * The token text is not null-terminated; use `text_len`.
 */
struct Token {
    enum Token_Kind kind;
    const char *text;
    size_t text_len;
    struct Location location;
};

/**
 * @struct Tokens
 * @brief Result of lexing an entire file.
 *
 * Owns 2 dynamic buffers:
 * - `content`: the concatenated file contents (with '\n' inserted after each
 *   line read by asm_get_line()).
 * - `elements`: the token array; each token's `text` points into `content`.
 *
 * @warning Because tokens reference `content.elements`, `content` must remain
 *          alive as long as tokens are used.
 */
struct Tokens {
    struct String content;
    size_t length;
    size_t capacity;
    struct Token* elements;
    size_t current_token;
    const char *current_key;
    size_t current_key_len;
    char *file_path;
};

/**
 * @brief Lexer state over a caller-provided input buffer.
 *
 * The lexer does not own `content`; the caller must keep it valid for the
 * lifetime of any tokens referencing it.
 *
 * Internal location tracking:
 * - `line_num` is 0-based internally (first line is 0).
 * - `begining_of_line` is the cursor index of the first character of the
 *   current line (used for column calculation).
 */
struct Lexer {
    const char * content;
    size_t content_len;
    size_t cursor;
    size_t line_num;
    size_t begining_of_line;
};

/**
 * @brief Operator/punctuation token table.
 *
 * The lexer uses this table to apply a longest-match rule for multi-character
 * operators (e.g. ">>=" over ">>" and ">").
 *
 * @note This table is defined in the header as `static`, so each translation
 * unit gets its own copy.
 */
static struct Literal_Token literal_tokens[] = {
    {.text = "("  , .kind = TOKEN_LPAREN}, 
    {.text = ")"  , .kind = TOKEN_RPAREN},
    {.text = "["  , .kind = TOKEN_LBRACKET}, 
    {.text = "]"  , .kind = TOKEN_RBRACKET},
    {.text = "{"  , .kind = TOKEN_LBRACE}, 
    {.text = "}"  , .kind = TOKEN_RBRACE},
    {.text = "#"  , .kind = TOKEN_HASH},
    {.text = "...", .kind = TOKEN_ELLIPSIS},
    {.text = "."  , .kind = TOKEN_DOT},
    {.text = ","  , .kind = TOKEN_COMMA},
    {.text = "?"  , .kind = TOKEN_QUESTION},
    {.text = ":"  , .kind = TOKEN_COLON},
    {.text = "==" , .kind = TOKEN_EQEQ},
    {.text = "!=" , .kind = TOKEN_NE},
    {.text = "="  , .kind = TOKEN_EQ},
    {.text = "!"  , .kind = TOKEN_BANG},
    {.text = ";"  , .kind = TOKEN_SEMICOLON},
    {.text = "\\" , .kind = TOKEN_BSLASH},
    {.text = "->" , .kind = TOKEN_ARROW},
    {.text = ">"  , .kind = TOKEN_GT},
    {.text = ">=" , .kind = TOKEN_GE},
    {.text = "<"  , .kind = TOKEN_LT},
    {.text = "<=" , .kind = TOKEN_LE},
    {.text = "<<=", .kind = TOKEN_LSHIFTEQ},
    {.text = ">>=", .kind = TOKEN_RSHIFTEQ},
    {.text = "++" , .kind = TOKEN_PLUSPLUS},
    {.text = "--" , .kind = TOKEN_MINUSMINUS},
    {.text = "<<" , .kind = TOKEN_LSHIFT},
    {.text = ">>" , .kind = TOKEN_RSHIFT},
    {.text = "+=", .kind = TOKEN_PLUSEQ},
    {.text = "-=", .kind = TOKEN_MINUSEQ},
    {.text = "*=", .kind = TOKEN_STAREQ},
    {.text = "/=", .kind = TOKEN_SLASHEQ},
    {.text = "%=", .kind = TOKEN_PERCENTEQ},
    {.text = "&=", .kind = TOKEN_ANDEQ},
    {.text = "|=", .kind = TOKEN_OREQ},
    {.text = "^=", .kind = TOKEN_XOREQ},
    {.text = "||", .kind = TOKEN_OROR},
    {.text = "&&", .kind = TOKEN_ANDAND},
    {.text = "|" , .kind = TOKEN_BITOR},
    {.text = "&" , .kind = TOKEN_BITAND},
    {.text = "^" , .kind = TOKEN_CARET},
    {.text = "~" , .kind = TOKEN_TILDE},
    {.text = "+" , .kind = TOKEN_PLUS},
    {.text = "-" , .kind = TOKEN_MINUS},
    {.text = "*" , .kind = TOKEN_STAR},
    {.text = "/" , .kind = TOKEN_SLASH},
    {.text = "%" , .kind = TOKEN_PERCENT},
};

#define literal_tokens_count (sizeof(literal_tokens) / sizeof(literal_tokens[0]))

/**
 * @brief List of keywords recognized by the lexer.
 *
 * If an identifier's spelling matches one of these strings exactly, the lexer
 * produces TOKEN_KEYWORD instead of TOKEN_IDENTIFIER.
 */
#ifndef AL_CPP_KEYWORDS
static const char * const keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
    "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
    "union", "unsigned", "void", "volatile", "while",
};
#else 
static const char * const keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "int", "long", "register",
    "return", "short", "signed", "sizeof", "static", "struct", "switch", "typedef",
    "union", "unsigned", "void", "volatile", "while", "alignas", "alignof", "and",
    "and_eq", "asm", "atomic_cancel", "atomic_commit", "atomic_noexcept", "bitand",
    "bitor", "bool", "catch", "char16_t", "char32_t", "char8_t", "class", "co_await",
    "co_return", "co_yield", "compl", "concept", "const_cast", "consteval", "constexpr",
    "constinit", "decltype", "delete", "dynamic_cast", "explicit", "export", "false",
    "friend", "inline", "mutable", "namespace", "new", "noexcept", "not", "not_eq",
    "nullptr", "operator", "or", "or_eq", "private", "protected", "public", "reflexpr",
    "reinterpret_cast", "requires", "static_assert", "static_cast", "synchronized",
    "template", "this", "thread_local", "throw", "true", "try", "typeid", "typename",
    "using", "virtual", "wchar_t", "xor", "xor_eq",
};
#endif /*AL_CPP_KEYWORDS*/
#define keywords_count (sizeof(keywords) / sizeof(keywords[0]))

/**
 * @def AL_UNUSED
 * @brief Mark a variable as intentionally unused.
 *
 * @param x Expression evaluated for side effects (if any) and then cast to
 *          void to suppress unused warnings.
 */
#define AL_UNUSED(x) (void)x

#define al_dprintERROR(fmt, ...) \
    fprintf(stderr, "\n%s:%d:\n[Error] in function '%s':\n        " \
    fmt "\n\n", __FILE__, __LINE__, __func__, __VA_ARGS__)

bool            al_is_identifier(char c);
bool            al_is_identifier_start(char c);
struct Tokens   al_lex_entire_file(char *file_path);
struct Lexer    al_lexer_alloc(const char *content, size_t len);
char            al_lexer_chop_char(struct Lexer *l);
void            al_lexer_chop_while(struct Lexer *l, bool (*pred)(char));
struct Token    al_lexer_next_token(struct Lexer *l);
bool            al_lexer_start_with(struct Lexer *l, const char *prefix);
void            al_lexer_trim_left(struct Lexer *l);
char            al_lexer_peek(const struct Lexer *l, size_t off);
void            al_token_print(struct Token tok);
const char *    al_token_kind_name(enum Token_Kind kind);
struct Tokens   al_tokens_alloc(void);
void            al_tokens_free(struct Tokens tokens);

#endif /*ALMOG_LEXER_H_*/

#ifdef ALMOG_LEXER_IMPLEMENTATION
#undef ALMOG_LEXER_IMPLEMENTATION

/**
 * @brief Returns whether @p c can appear in an identifier after the first
 * character.
 *
 * Matches the implementation: alphanumeric (per `asm_isalnum`) or underscore.
 *
 * @param c Character to test.
 * @return true if @p c is valid as a non-initial identifier character.
 */
bool al_is_identifier(char c)
{
    return asm_isalnum(c) || c == '_';
}

/**
 * @brief Returns whether @p c can start an identifier.
 *
 * Matches the implementation: alphabetic (per `asm_isalpha`) or underscore.
 *
 * @param c Character to test.
 * @return true if @p c is valid as an initial identifier character.
 */
bool al_is_identifier_start(char c)
{
    return asm_isalpha(c) || c == '_';
}

struct Tokens al_lex_entire_file(char *file_path)
{
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        al_dprintERROR("Could not open file '%s'\n", file_path);
        exit(1);
    }

    struct Tokens tokens = al_tokens_alloc();
    asm_strncpy(tokens.file_path, file_path, ASM_MAX_LEN);

    char temp_str[ASM_MAX_LEN];
    int len = 0;
    while ((len = asm_get_line(fp, temp_str)) != EOF) {
        for (int i = 0; i < len; i++) {
            ada_appand(char, tokens.content, temp_str[i]);
        }
        ada_appand(char, tokens.content, '\n');
    }

    struct Lexer l = al_lexer_alloc(tokens.content.elements, tokens.content.length);

    struct Token t = al_lexer_next_token(&l);
    while (t.kind != TOKEN_EOF) {
        ada_appand(struct Token, tokens, t);
        t = al_lexer_next_token(&l);
    }
    ada_appand(struct Token, tokens, t);

    return tokens;
}

/**
 * @brief Create a lexer over an input buffer.
 *
 * Initializes cursor and location state to the beginning of the buffer.
 * No memory is allocated; the lexer holds only pointers/indices.
 *
 * @param content Pointer to the input text (need not be null-terminated).
 * @param len Length of @p content in bytes.
 * @return A lexer initialized to the start of @p content.
 */
struct Lexer al_lexer_alloc(const char *content, size_t len)
{
    struct Lexer l = {0};
    l.content = content;
    l.content_len = len;
    return l;
}

/**
 * @brief Consume and return the next character from the input.
 *
 * Advances the lexer's cursor by 1. If the consumed character is a newline
 * (`'\\n'`), the lexer's internal line/column bookkeeping is updated:
 * - `line_num` is incremented
 * - `begining_of_line` is set to the new cursor position
 *
 * @param l Lexer to advance.
 * @return The consumed character.
 *
 * @pre `l->cursor < l->content_len` (enforced via @c AL_ASSERT in the
 *      implementation).
 */
char al_lexer_chop_char(struct Lexer *l)
{
    AL_ASSERT(l->cursor < l->content_len);
    char c = l->content[l->cursor++]; 
    if (c == '\n') {
        l->line_num++;
        l->begining_of_line = l->cursor;
    }
    return c;
}

/**
 * @brief Consume characters while @p pred returns true for the next character.
 *
 * Uses @ref al_lexer_chop_char internally, so newline bookkeeping is applied.
 *
 * @param l Lexer to advance.
 * @param pred Predicate called with the next character to decide whether to
 *             consume it.
 */
void al_lexer_chop_while(struct Lexer *l, bool (*pred)(char))
{
    while (l->cursor < l->content_len && pred(l->content[l->cursor])) {
        al_lexer_chop_char(l);
    }
}

/**
 * @brief Return the next token from the input and advance the lexer.
 *
 * This function first calls @ref al_lexer_trim_left, so leading whitespace is
 * skipped (including newlines).
 *
 * The returned token:
 * - has `text` pointing into the original buffer at the token start
 * - has `text_len` equal to the number of bytes consumed for the token
 * - has 1-based `location.line_num` and 1-based `location.col`
 *
 * Tokenization behavior matches the implementation:
 * - End of input => @c TOKEN_EOF
 * - Preprocessor directive: a `#` at column 1 (after trimming) consumes until
 *   newline (and includes the newline if present) => @c TOKEN_PP_DIRECTIVE
 * - Identifiers: `[A-Za-z_][A-Za-z0-9_]*` => @c TOKEN_IDENTIFIER, upgraded to
 *   @c TOKEN_KEYWORD if it matches an entry in `keywords[]`
 * - String literal: starts with `"` and consumes until the next `"` or newline
 *   (includes the closing `"` if present) => @c TOKEN_STRING_LIT
 * - Character literal: starts with `'` and consumes until the next `'` or
 *   newline (includes the closing `'` if present) => @c TOKEN_CHAR_LIT
 * - Line comment: starts with `//` and consumes until newline (and includes the
 *   newline if present) => @c TOKEN_COMMENT
 * - Block comment: starts with `/ *` and consumes until the first `*\ /` (includes
 *   the final `/`), or until end of input => @c TOKEN_COMMENT
 * - Number literals:
 *   - decimal integers/floats with optional exponent (`e`/`E`)
 *   - hex integers and hex floats (hex float requires `p`/`P` exponent when a
 *     fractional part is present)
 *   - binary integers with `0b`/`0B`
 *   - explicit octal integers with `0o`/`0O`
 *   - accepts common integer suffixes (`uUlLzZ`) and float suffixes (`fFlL`)
 *   - certain malformed forms are returned as @c TOKEN_INVALID
 * - Otherwise: matches the longest operator/punctuation from `literal_tokens[]`
 *   (longest-match rule) and returns its kind
 * - If nothing matches, consumes one character and returns @c TOKEN_INVALID
 *
 * @warning Escape sequences in string/character literals are not interpreted;
 *          a quote character ends the literal even if preceded by a backslash.
 *
 * @param l Lexer to tokenize from.
 * @return The next token.
 */
struct Token al_lexer_next_token(struct Lexer *l)
{
    al_lexer_trim_left(l);

    struct Token token = {
        .kind = TOKEN_INVALID,
        .text = &(l->content[l->cursor]),
        .text_len = 0,
        .location.line_num = l->line_num+1,
        .location.col = l->cursor - l->begining_of_line+1,
    };
    size_t start = l->cursor;

    if (l->cursor >= l->content_len) {
        token.kind = TOKEN_EOF;
    } else if (l->content[l->cursor] == '#' && token.location.col == 1) {
        token.kind = TOKEN_PP_DIRECTIVE;
        for (;l->cursor < l->content_len && l->content[l->cursor] != '\n';) {
            al_lexer_chop_char(l);
        }
        if (l->cursor < l->content_len) {
            al_lexer_chop_char(l);
        }
    } else if (al_is_identifier_start(l->content[l->cursor])) {
        token.kind = TOKEN_IDENTIFIER;
        for ( ; l->cursor < l->content_len && al_is_identifier(l->content[l->cursor]); ) {
            al_lexer_chop_char(l);
        }
        {
            size_t ident_len = l->cursor - start;
            for (size_t i = 0; i < keywords_count; i++) {
                size_t kw_len = asm_length(keywords[i]);
                if (ident_len == kw_len && asm_strncmp(token.text, keywords[i], kw_len)) {
                    token.kind = TOKEN_KEYWORD;
                    break;
                }
            }
        }
    } else if (l->content[l->cursor] == '"') {
        token.kind = TOKEN_STRING_LIT;
        al_lexer_chop_char(l);
        token.text++;
        start = l->cursor+1;

        for ( ; (l->cursor < l->content_len) && (l->content[l->cursor] != '"') && (l->content[l->cursor] != '\n'); ) {
            al_lexer_chop_char(l);
        }
        if ((l->cursor < l->content_len) && (l->content[l->cursor] == '"')) {
            al_lexer_chop_char(l);
        }
    } else if (l->content[l->cursor] == '\'') {
        token.kind = TOKEN_CHAR_LIT;
        al_lexer_chop_char(l);
        token.text++;
        start = l->cursor+1;

        for ( ; (l->cursor < l->content_len) && (l->content[l->cursor] != '\'') && (l->content[l->cursor] != '\n'); ) {
            al_lexer_chop_char(l);
        }
        if ((l->cursor < l->content_len) && (l->content[l->cursor] == '\'')) {
            al_lexer_chop_char(l);
        }
    } else if (al_lexer_start_with(l, "//")) {
        token.kind = TOKEN_COMMENT;
        for (;l->cursor < l->content_len && l->content[l->cursor] != '\n';) {
            al_lexer_chop_char(l);
        }
        if (l->cursor < l->content_len) {
            al_lexer_chop_char(l);
        }
    } else if (al_lexer_start_with(l, "/*")) {
        token.kind = TOKEN_COMMENT;
        al_lexer_chop_char(l);
        al_lexer_chop_char(l);
        for ( ; l->cursor < l->content_len; ) {
            if ((l->content[l->cursor-1] == '*') && (l->content[l->cursor] == '/')) {
                al_lexer_chop_char(l);
                break;
            }
            al_lexer_chop_char(l);
        }
    } else if (asm_isdigit(l->content[l->cursor]) || (l->content[l->cursor] == '.' && asm_isdigit(al_lexer_peek(l, 1)))) {
        token.kind = TOKEN_INT_LIT_DEC;
        bool is_float = false;
        bool invalid = false;

        if (l->content[l->cursor] == '.') {
            token.kind = TOKEN_FLOAT_LIT_DEC;
            is_float = true;
            al_lexer_chop_char(l);
            al_lexer_chop_while(l, asm_isdigit);

            /* optional exponent */
            if (al_lexer_peek(l, 0) == 'e' || al_lexer_peek(l, 0) == 'E') {
                is_float = true;
                al_lexer_chop_char(l);
                if (al_lexer_peek(l, 0) == '+' || al_lexer_peek(l, 0) == '-') {
                    al_lexer_chop_char(l);
                }
                if (!asm_isdigit(al_lexer_peek(l, 0))) {
                    invalid = true; /* ".5e" or ".5e+" */
                }
                al_lexer_chop_while(l, asm_isdigit);
            }
        } else {
            /* starts with digit */
            if (al_lexer_peek(l, 0) == '0' && (al_lexer_peek(l, 1) == 'x' || al_lexer_peek(l, 1) == 'X')) {
                token.kind = TOKEN_INT_LIT_HEX;
                al_lexer_chop_char(l);
                al_lexer_chop_char(l);

                size_t mantissa_digits = 0;
                while (asm_isXdigit(al_lexer_peek(l, 0)) || asm_isxdigit(al_lexer_peek(l, 0))) {
                    mantissa_digits++;
                    al_lexer_chop_char(l);
                }
                if (al_lexer_peek(l, 0) == '.') {
                    token.kind = TOKEN_FLOAT_LIT_HEX;
                    is_float = true;
                    al_lexer_chop_char(l);
                    while (asm_isXdigit(al_lexer_peek(l, 0)) || asm_isxdigit(al_lexer_peek(l, 0))) {
                        mantissa_digits++;
                        al_lexer_chop_char(l);
                    }
                }
                if (mantissa_digits == 0) {
                    invalid = true; /* "0x" or "0x." */
                }

                /* Hex float requires p/P exponent if it's a float form. */
                if (al_lexer_peek(l, 0) == 'p' || al_lexer_peek(l, 0) == 'P') {
                    is_float = true;
                    al_lexer_chop_char(l);
                    if (al_lexer_peek(l, 0) == '+' || al_lexer_peek(l, 0) == '-') {
                        al_lexer_chop_char(l);
                    }
                    if (!asm_isdigit(al_lexer_peek(l, 0))) {
                        invalid = true; /* "0x1.fp" / "0x1p+" */
                    }
                    al_lexer_chop_while(l, asm_isdigit);
                } else if (is_float) {
                    /* Had a '.' in hex mantissa but no p-exponent => invalid hex float */
                    invalid = true;
                }
            } else if (al_lexer_peek(l, 0) == '0' && (al_lexer_peek(l, 1) == 'b' || al_lexer_peek(l, 1) == 'B')) {
                token.kind = TOKEN_INT_LIT_BIN;
                al_lexer_chop_char(l);
                al_lexer_chop_char(l);
                if (!asm_isbdigit(al_lexer_peek(l, 0))) {
                    invalid = true; /* "0b" */
                }
                al_lexer_chop_while(l, asm_isbdigit);
            } else if (al_lexer_peek(l, 0) == '0' && (al_lexer_peek(l, 1) == 'o' || al_lexer_peek(l, 1) == 'O')) {
                token.kind = TOKEN_INT_LIT_OCT;
                al_lexer_chop_char(l);
                al_lexer_chop_char(l);
                if (!asm_isodigit(al_lexer_peek(l, 0))) {
                    invalid = true; /* "0o" */
                }
                while (asm_isodigit(al_lexer_peek(l, 0))) {
                    al_lexer_chop_char(l);
                }
            } else {
                token.kind = TOKEN_INT_LIT_DEC;
                al_lexer_chop_while(l, asm_isdigit);

                if (al_lexer_peek(l, 0) == '.') {
                    token.kind = TOKEN_FLOAT_LIT_DEC;
                    is_float = true;
                    al_lexer_chop_char(l);
                    al_lexer_chop_while(l, asm_isdigit);
                }

                if (al_lexer_peek(l, 0) == 'e' || al_lexer_peek(l, 0) == 'E') {
                    is_float = true;
                    al_lexer_chop_char(l);
                    if (al_lexer_peek(l, 0) == '+' || al_lexer_peek(l, 0) == '-') {
                        al_lexer_chop_char(l);
                    }
                    if (!asm_isdigit(al_lexer_peek(l, 0))) {
                        invalid = true; /* "1e" / "1e+" */
                    }
                    al_lexer_chop_while(l, asm_isdigit);
                }
            }
        }

        /* Suffix handling */
        if (is_float) {
            /* float suffixes: f/F/l/L (accept at most one, but we’ll be permissive) */
            while (al_lexer_peek(l, 0) == 'f' || al_lexer_peek(l, 0) == 'F' ||
                   al_lexer_peek(l, 0) == 'l' || al_lexer_peek(l, 0) == 'L') {
                al_lexer_chop_char(l);
            }
        } else {
            /* integer suffixes: u/U/l/L/z/Z (permissive) */
            while (al_lexer_peek(l, 0) == 'u' || al_lexer_peek(l, 0) == 'U' ||
                   al_lexer_peek(l, 0) == 'l' || al_lexer_peek(l, 0) == 'L' ||
                   al_lexer_peek(l, 0) == 'z' || al_lexer_peek(l, 0) == 'Z') {
                al_lexer_chop_char(l);
            }
        }

        if (invalid) token.kind = TOKEN_INVALID;
    } else {
        size_t longest_matching_token = 0;
        enum Token_Kind best_kind = TOKEN_INVALID;
        for (size_t i = 0; i < literal_tokens_count; i++) {
            if (al_lexer_start_with(l, literal_tokens[i].text)) {
                /* NOTE: assumes that literal_tokens[i].text does not have any '\n' */
                size_t text_len = asm_length(literal_tokens[i].text);
                if (text_len > longest_matching_token) {
                    longest_matching_token = text_len;
                    best_kind = literal_tokens[i].kind;                }
            }
        }
        if (longest_matching_token > 0) {
            token.kind = best_kind;
            for (size_t i = 0; i < longest_matching_token; i++) {
                al_lexer_chop_char(l);
            }
        } else {
            token.kind = TOKEN_INVALID;
            al_lexer_chop_char(l);
        }
    }

    token.text_len = l->cursor - start;

    return token;
}

/**
 * @brief Check whether the remaining input at the current cursor starts with
 * @p prefix.
 *
 * @param l Lexer whose input is tested.
 * @param prefix Null-terminated prefix string to match.
 * @return true if @p prefix is empty or fully matches at the current cursor;
 *         false otherwise.
 */
bool al_lexer_start_with(struct Lexer *l, const char *prefix)
{
    size_t prefix_len = asm_length(prefix); 
    if (prefix_len == 0) {
        return true;
    }
    if (l->cursor + prefix_len > l->content_len) {
        return false;
    }
    for (size_t i = 0; i < prefix_len; i++) {
        if (prefix[i] != l->content[l->cursor + i]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Consume leading whitespace characters.
 *
 * Whitespace is defined by `asm_isspace` from "Almog_String_Manipulation.h".
 * Uses @ref al_lexer_chop_char, so newlines update line/column bookkeeping.
 *
 * @param l Lexer to advance.
 */
void al_lexer_trim_left(struct Lexer *l)
{
    for (;l->cursor < l->content_len;) {
        if (!asm_isspace(l->content[l->cursor])) {
            break;
        }
        al_lexer_chop_char(l);
    }
}

/**
 * @brief Peek at a character in the input without advancing the lexer.
 *
 * @param l Lexer to read from.
 * @param off Offset from the current cursor (0 means current character).
 * @return The character at `cursor + off`, or `'\\0'` if out of range.
 */
char al_lexer_peek(const struct Lexer *l, size_t off)
{
    size_t i = l->cursor + off;
    if (i >= l->content_len) return '\0';
    return l->content[i];
}

/**
 * @brief Print a human-readable representation of @p tok to stdout.
 *
 * Output format matches the implementation:
 * `line:col:(KIND) -> "TEXT"`
 *
 * @note The token text is printed using a precision specifier (`%.*s`) and does
 *       not need to be null-terminated.
 *
 * @param tok Token to print.
 */
void al_token_print(struct Token tok)
{
    printf("%4zu:%-3zu:(%-19s) -> \"%.*s\"\n", tok.location.line_num, tok.location.col, al_token_kind_name(tok.kind), (int)tok.text_len, tok.text);
}

/**
 * @brief Convert a token kind enum to a stable string name.
 *
 * The returned pointer refers to a string literal.
 *
 * @param kind Token kind.
 * @return A string name such as `"TOKEN_IDENTIFIER"`, or asserts on unknown
 *         kinds in the implementation's default case.
 */
const char *al_token_kind_name(enum Token_Kind kind)
{
    switch (kind) {
        case TOKEN_EOF:
            return ("TOKEN_EOF");
        case TOKEN_INVALID:
            return ("TOKEN_INVALID");
        case TOKEN_PP_DIRECTIVE:
            return ("TOKEN_PP_DIRECTIVE");
        case TOKEN_IDENTIFIER:
            return ("TOKEN_IDENTIFIER");
        case TOKEN_LPAREN:
            return ("TOKEN_LPAREN");
        case TOKEN_RPAREN:
            return ("TOKEN_RPAREN");
        case TOKEN_LBRACKET:
            return ("TOKEN_LBRACKET");
        case TOKEN_RBRACKET:
            return ("TOKEN_RBRACKET");
        case TOKEN_LBRACE:
            return ("TOKEN_LBRACE");
        case TOKEN_RBRACE:
            return ("TOKEN_RBRACE");
        case TOKEN_DOT:
            return ("TOKEN_DOT");
        case TOKEN_COMMA:
            return ("TOKEN_COMMA");
        case TOKEN_SEMICOLON:
            return ("TOKEN_SEMICOLON");
        case TOKEN_BSLASH:
            return ("TOKEN_BSLASH");
        case TOKEN_QUESTION:
            return ("TOKEN_QUESTION");
        case TOKEN_COLON:
            return ("TOKEN_COLON");
        case TOKEN_LT:
            return ("TOKEN_LT");
        case TOKEN_GT:
            return ("TOKEN_GT");
        case TOKEN_GE:
            return ("TOKEN_GE");
        case TOKEN_LE:
            return ("TOKEN_LE");
        case TOKEN_KEYWORD:
            return ("TOKEN_KEYWORD");
        case TOKEN_INT_LIT_BIN:
            return ("TOKEN_INT_LIT_BIN");
        case TOKEN_INT_LIT_OCT:
            return ("TOKEN_INT_LIT_OCT");
        case TOKEN_INT_LIT_DEC:
            return ("TOKEN_INT_LIT_DEC");
        case TOKEN_INT_LIT_HEX:
            return ("TOKEN_INT_LIT_HEX");
        case TOKEN_FLOAT_LIT_DEC:
            return ("TOKEN_FLOAT_LIT_DEC");
        case TOKEN_FLOAT_LIT_HEX:
            return ("TOKEN_FLOAT_LIT_HEX");
        case TOKEN_COMMENT:
            return ("TOKEN_COMMENT");
        case TOKEN_STRING_LIT:
            return ("TOKEN_STRING_LIT");
        case TOKEN_CHAR_LIT:
            return ("TOKEN_CHAR_LIT");
        case TOKEN_EQ:
            return ("TOKEN_EQ");
        case TOKEN_EQEQ:
            return ("TOKEN_EQEQ");
        case TOKEN_NE:
            return ("TOKEN_NE");
        case TOKEN_BANG:
            return ("TOKEN_BANG");
        case TOKEN_BITAND:
            return ("TOKEN_BITAND");
        case TOKEN_ANDAND:
            return ("TOKEN_ANDAND");
        case TOKEN_BITOR:
            return ("TOKEN_BITOR");
        case TOKEN_OROR:
            return ("TOKEN_OROR");
        case TOKEN_CARET:
            return ("TOKEN_CARET");
        case TOKEN_TILDE:
            return ("TOKEN_TILDE");
        case TOKEN_PLUSPLUS:
            return ("TOKEN_PLUSPLUS");
        case TOKEN_MINUSMINUS:
            return ("TOKEN_MINUSMINUS");
        case TOKEN_LSHIFT:
            return ("TOKEN_LSHIFT");
        case TOKEN_RSHIFT:
            return ("TOKEN_RSHIFT");
        case TOKEN_PLUS:
            return ("TOKEN_PLUS");
        case TOKEN_MINUS:
            return ("TOKEN_MINUS");
        case TOKEN_STAR:
            return ("TOKEN_STAR");
        case TOKEN_SLASH:
            return ("TOKEN_SLASH");
        case TOKEN_HASH:
            return ("TOKEN_HASH");
        case TOKEN_PERCENT:
            return ("TOKEN_PERCENT");
        case TOKEN_PLUSEQ:
            return ("TOKEN_PLUSEQ");
        case TOKEN_MINUSEQ:
            return ("TOKEN_MINUSEQ");
        case TOKEN_STAREQ:
            return ("TOKEN_STAREQ");
        case TOKEN_SLASHEQ:
            return ("TOKEN_SLASHEQ");
        case TOKEN_PERCENTEQ:
            return ("TOKEN_PERCENTEQ");
        case TOKEN_ANDEQ:
            return ("TOKEN_ANDEQ");
        case TOKEN_OREQ:
            return ("TOKEN_OREQ");
        case TOKEN_XOREQ:
            return ("TOKEN_XOREQ");
        case TOKEN_LSHIFTEQ:
            return ("TOKEN_LSHIFTEQ");
        case TOKEN_RSHIFTEQ:
            return ("TOKEN_RSHIFTEQ");
        case TOKEN_ARROW:
            return ("TOKEN_ARROW");
        case TOKEN_ELLIPSIS:
            return ("TOKEN_ELLIPSIS");
        default:
            AL_ASSERT(0 && "Unknown kind");
    }
    return NULL;
}

struct Tokens al_tokens_alloc(void)
{
    struct Tokens tokens = {0};
    ada_init_array(struct Token, tokens);
    ada_init_array(char, tokens.content);
    tokens.current_token = 0;
    tokens.file_path = (char *)malloc(sizeof(char) * ASM_MAX_LEN);

    return tokens;
}

void al_tokens_free(struct Tokens tokens)
{
    AL_FREE(tokens.content.elements);   
    AL_FREE(tokens.elements);   
    AL_FREE(tokens.file_path);
}

#endif /*ALMOG_LEXER_IMPLEMENTATION*/

