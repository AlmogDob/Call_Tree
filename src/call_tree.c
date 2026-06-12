#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

#define ALMOG_LEXER_IMPLEMENTATION
#define ALMOG_STRING_MANIPULATION_IMPLEMENTATION
#include "Almog_Lexer.h"



int main(int argc, char const *argv[])
{
    if (argc != 2) {
        printf("Usage: %s 'entry_file.c' 'function_name'\n", *(argv++));
        printf("got argc == %d\n",argc);
        asm_dprintSTRING(*(argv));
        return 0;
    }
    char *entry_file = *(++argv);
    struct Tokens tokens = al_lex_entire_file(entry_file);
    
    for (size_t i = 0; i < tokens.length; i++) {
        struct Token token = tokens.elements[i];
        if (token.kind == TOKEN_PP_DIRECTIVE) {
            al_token_print(tokens.elements[i]);
        }
    }

    char current_working_directory[ASM_MAX_LEN];
    if (getcwd(current_working_directory, sizeof(current_working_directory)) == NULL) {
        al_dprintERROR("%s", "Could not get current working directory.");
        return -1;
    }

    asm_dprintSTRING(current_working_directory);
    asm_dprintSTRING(entry_file);









    al_tokens_free(tokens);
    return 0;
}
