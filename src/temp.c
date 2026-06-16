#define ALMOG_STRING_MANIPULATION_IMPLEMENTATION
#define ALMOG_LEXER_IMPLEMENTATION
#include "./includes/Almog\
_Lexer.h"

int main(void)
{
    struct Tokens tokens = al_lex_entire_file("../src/temp.c");
    
    for (size_t i = 0; ((i) < tokens.length); i++) {
        al_token_print(tokens.elements[i]);
    }
    asm_dprintSIZE_T(tokens.length);

    al_tokens_free(tokens);

    return 0;
}
