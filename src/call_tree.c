#include <stdio.h>

#define ALMOG_LEXER_IMPLEMENTATION
#define ALMOG_STRING_MANIPULATION_IMPLEMENTATION
#include "./includes/Almog_Lexer.h"

#define APM_MAX_LEN ASM_MAX_LEN
#define ALMOG_PATH_MANIPULATION_IMPLEMENTATION
#include "./includes/Almog_Path_Manipulation.h"

#define SUCCESS APM_SUCCESS
#define FAIL APM_FAIL

struct Lexed_Files {
    size_t length;
    size_t capacity;
    struct Tokens *elements;
};

struct Function_Definition {
    char name[ASM_MAX_LEN];
    char file_name[ASM_MAX_LEN];
    int file_index;
    size_t token_start_index;
    size_t token_end_index;
    size_t LPAREN_index;
    size_t RPAREN_index;
    size_t LBRACE_index;
    size_t RBRACE_index;
};

struct Func_Def_Array {
    size_t length;
    size_t capacity;
    struct Function_Definition *elements;
};

struct Apm_Word_Array word_array_alloc() 
{
    struct Apm_Word_Array word_array = {
        .length = 0,
        .elements = (Apm_Word *)AL_MALLOC(APM_WORD_ARRAY_MAX_LEN * sizeof(Apm_Word))
    };
    if (word_array.elements == NULL) {
        al_dprintERROR("%s", "Failed to allocate a word array.");
        exit(1);
    }

    return word_array;
}

#define word_array_print(word_array) do {al_dprintINFO("%s = ", #word_array); word_array_print_imp(word_array, 7);} while (0)
void word_array_print_imp(struct Apm_Word_Array wa, size_t padding)
{
    for (size_t i = 0; i < wa.length; i++) {
        printf("%*.s%s\n", (int)padding, "", wa.elements[i]);
    }
}

#define function_definition_print(tokens, func_def) do {al_dprintINFO("%s = ", #func_def); function_definition_print_imp(tokens, func_def, 11);} while (0)
void function_definition_print_imp(struct Tokens tokens, struct Function_Definition func_def, size_t padding)
{
    printf("%*.sname              -> %s\n", (int)padding, "", func_def.name);
    printf("%*.sfile name         -> %s:%zu\n", (int)padding, "", func_def.file_name, tokens.elements[func_def.LPAREN_index].location.line_num);
    printf("%*.sfile index        -> %d\n", (int)padding, "", func_def.file_index);
    printf("%*.stoken start index -> %-4zu = ", (int)padding, "", func_def.token_start_index);
    al_token_print(tokens.elements[func_def.token_start_index]);
    printf("%*.stoken end index   -> %-4zu = ", (int)padding, "", func_def.token_end_index);
    al_token_print(tokens.elements[func_def.token_end_index]);
    printf("%*.sLPAREN index      -> %-4zu = ", (int)padding, "", func_def.LPAREN_index);
    al_token_print(tokens.elements[func_def.LPAREN_index]);
    printf("%*.sRPAREN index      -> %-4zu = ", (int)padding, "", func_def.RPAREN_index);
    al_token_print(tokens.elements[func_def.RPAREN_index]);
    printf("%*.sLBRACE index      -> %-4zu = ", (int)padding, "", func_def.LBRACE_index);
    al_token_print(tokens.elements[func_def.LBRACE_index]);
    printf("%*.sRBRACE index      -> %-4zu = ", (int)padding, "", func_def.RBRACE_index);
    al_token_print(tokens.elements[func_def.RBRACE_index]);
}

#define func_def_array_print(lexed_files, func_def_array) do {al_dprintINFO("%s = ", #func_def_array); func_def_array_print_imp(lexed_files, func_def_array, 11);} while (0)
void func_def_array_print_imp (struct Lexed_Files lexed_files, struct Func_Def_Array func_def_array, size_t padding)
{
    for (size_t i = 0; i < func_def_array.length; i++) {
        struct Function_Definition func_def = func_def_array.elements[i];
        printf("%*.sFunc No.%zu:\n", (int)padding, "", i);
        printf("%*.s    name       -> %s\n", (int)padding, "", func_def.name);
        printf("%*.s    file name  -> %s:%zu\n", (int)padding, "", func_def.file_name, lexed_files.elements[func_def.file_index].elements[func_def.LPAREN_index].location.line_num);
        printf("%*.s    file index -> %d\n", (int)padding, "", func_def.file_index);
    }
}

bool include_paths_get_from_tokens(struct Apm_Word_Array *word_array, struct Tokens tokens)
{
    Apm_Word temp_word;
    Apm_Word current_directive;
    for (size_t i = 0; i < tokens.length; i++) {
        struct Token token = tokens.elements[i];
        if (token.kind == TOKEN_PP_DIRECTIVE) {
            asm_strncpy(current_directive, token.text, token.text_len);
            asm_get_token_and_cut(temp_word, current_directive, '"', true);
            asm_strip_whitespace(temp_word);
            if (asm_strncmp(temp_word, "#include", ASM_MAX_LEN)) {
                if (current_directive[0] == '"') {
                    asm_get_token_and_cut(temp_word, current_directive, '"', false);
                    asm_get_token_and_cut(temp_word, current_directive, '"', false);
                    if (APM_FAIL == apm_path_fix(temp_word)) {
                        al_dprintERROR("Could not fix path '%s'.", temp_word);
                        return FAIL;
                    }
                    if (word_array->length < APM_WORD_ARRAY_MAX_LEN) {
                        asm_strncpy(word_array->elements[word_array->length++], temp_word, ASM_MAX_LEN);
                    } else {
                        al_dprintERROR("Could not add word to word array. Capacity is %d and the current length is %zu", APM_WORD_ARRAY_MAX_LEN, word_array->length);
                        return FAIL;
                    }
                }
            }
        }
    }
    return SUCCESS;
}

bool paths_add_prefix(struct Apm_Word_Array path_array, char * prefix)
{
    for (size_t i = 0; i < path_array.length; i++) {
        Apm_Word current_word;
        Apm_Word joined;

        asm_strncpy(current_word, path_array.elements[i], ASM_MAX_LEN);
        if (APM_FAIL == apm_join_two_paths(joined, prefix, current_word)) {
            al_dprintERROR("Could not join path2 '%s' to path1 '%s'.", current_word, prefix);
            return FAIL;
        }
        asm_strncpy(path_array.elements[i], joined, ASM_MAX_LEN);
    }

    return SUCCESS;
}

bool includes_path_get_from_lexed_file(struct Apm_Word_Array *include_paths, struct Tokens lexed_file)
{
    if (FAIL == include_paths_get_from_tokens(include_paths, lexed_file)) {
        al_dprintERROR("Failed to get the include paths from the tokens of '%s'.", lexed_file.file_path);
        return FAIL;
    }
    Apm_Word entry_file_absolute_dir;
    if (APM_FAIL == apm_directory_get_from_path(entry_file_absolute_dir, lexed_file.file_path))
    {
        al_dprintERROR("Failed to get the absolute path of '%s'", lexed_file.file_path);
        return FAIL;
    }
    if (FAIL == paths_add_prefix(*include_paths, entry_file_absolute_dir)) {
        al_dprintERROR("Could not add prefix '%s' to paths.", entry_file_absolute_dir);
        return FAIL;
    }

    return SUCCESS;
}

bool path_is_in_lexed_files(struct Lexed_Files lexed_files, char *path)
{
    for (size_t i = 0; i < lexed_files.length; i++) {
        if (asm_strncmp(path, lexed_files.elements[i].file_path, ASM_MAX_LEN)) {
            return SUCCESS;
        }
    }

    return FAIL;
}

bool lex_entire_file_recursively(struct Lexed_Files *lexed_files, char *path)
{
    if (APM_FAIL == apm_path_exists(path)) {
        al_dprintERROR("Path does not exist: '%s'", path);
        return FAIL;
    }
    if (APM_FAIL == apm_path_is_absolute(path)) {
        al_dprintERROR("Inputted path is not absolute '%s'.", path);
        return FAIL;
    }
    if (APM_SUCCESS == apm_path_is_directory(path)) {
        al_dprintERROR("Expected a file, but got a directory: '%s'.", path);
        return FAIL;
    }
    struct Tokens tokens = al_lex_entire_file(path);
    ada_appand(struct Tokens, *lexed_files, tokens);
    struct Apm_Word_Array nested_include_paths = {0};
    nested_include_paths = word_array_alloc();
    if (FAIL == includes_path_get_from_lexed_file(&nested_include_paths, tokens)) {
        al_dprintERROR("Could not get includes path from file '%s'.", tokens.file_path);
        AL_FREE(nested_include_paths.elements);
        return FAIL;
    }
    for (size_t i = 0; i < nested_include_paths.length; i++) {
        if (FAIL == path_is_in_lexed_files(*lexed_files, nested_include_paths.elements[i])) {
            if (FAIL == lex_entire_file_recursively(lexed_files, nested_include_paths.elements[i])) {
                al_dprintERROR("Could not lex recursively file '%s'.", nested_include_paths.elements[i]);
                AL_FREE(nested_include_paths.elements);
                return FAIL;
            }
        } else {
            al_dprintWARNING("Detected repeated inclusion. File: '%s' includes '%s' that was already encountered.", path, nested_include_paths.elements[i]);
        }
    }
    // al_dprintINFO("In file %s:", path);
    // word_array_print(nested_include_paths);

    AL_FREE(nested_include_paths.elements);
    return SUCCESS;
}

bool LPAREN_find_matching_RPAREN(struct Tokens tokens, size_t LPAREN_index, size_t *matching_RPAREN_index, bool to_log)
{
    if (tokens.elements[LPAREN_index].kind != TOKEN_LPAREN) {
        if (to_log) al_dprintERROR("Inputted token index does not match a LPAREN. Token kind = %s", al_token_kind_name(tokens.elements[LPAREN_index].kind));
        return FAIL;
    }

    int counter = 1;
    for (size_t i = LPAREN_index + 1; i < tokens.length; i++) {
        struct Token current_token = tokens.elements[i];
        if (current_token.kind == TOKEN_LPAREN) {
            counter++;
        } else if (current_token.kind == TOKEN_RPAREN) {
            counter--;
            if (counter == 0) {
                /* found matching RPAREN */
                if (matching_RPAREN_index) *matching_RPAREN_index = i;
                return SUCCESS;
            }
        }
    }

    return FAIL;
}

bool LBRACE_find_matching_RBRACE(struct Tokens tokens, size_t LBRACE_index, size_t *matching_RBRACE_index, bool to_log)
{
    if (tokens.elements[LBRACE_index].kind != TOKEN_LBRACE) {
        if (to_log) al_dprintERROR("Inputted token index does not match a LBRACE. Token kind = %s", al_token_kind_name(tokens.elements[LBRACE_index].kind));
        return FAIL;
    }

    int counter = 1;
    for (size_t i = LBRACE_index + 1; i < tokens.length; i++) {
        struct Token current_token = tokens.elements[i];
        if (current_token.kind == TOKEN_LBRACE) {
            counter++;
        } else if (current_token.kind == TOKEN_RBRACE) {
            counter--;
            if (counter == 0) {
                /* found matching RBRACE */
                if (matching_RBRACE_index) *matching_RBRACE_index = i;
                return SUCCESS;
            }
        }
    }

    return FAIL;
}

bool token_sequence_at_start_index_is_function_definition(struct Tokens tokens, size_t start_index, struct Function_Definition *func_def, bool to_log) {
    if (start_index + 1 >= tokens.length) {
        if (to_log) al_dprintERROR("Start index + 1 (%zu) is bigger than tokens.length %zu.", start_index + 1, tokens.length);
        return FAIL;
    }

    struct Token start_token = tokens.elements[start_index];
    if (start_token.kind != TOKEN_IDENTIFIER) {
        if (to_log) al_dprintERROR("Token at start index (%zu) of file '%s' is not an IDENTIFIER."
            " The token at index %zu: %4zu:%-3zu:(%-19s) -> \"%.*s\".", start_index, tokens.file_path, start_index, start_token.location.line_num,
            start_token.location.col, al_token_kind_name(start_token.kind), (int)start_token.text_len, start_token.text);
        return FAIL;
    }
    size_t LPAREN_index_candidate = start_index + 1;
    while (tokens.elements[LPAREN_index_candidate].kind == TOKEN_COMMENT) {
        LPAREN_index_candidate++;
        if (LPAREN_index_candidate >= tokens.length) {
            if (to_log) al_dprintERROR("%s", "Could not find LPAREN token.");
            return FAIL;
        }
    }
    struct Token LPAREN_token_candidate = tokens.elements[LPAREN_index_candidate];
    if (LPAREN_token_candidate.kind != TOKEN_LPAREN) {
        if (to_log) al_dprintERROR("Token at LPAREN index candidate (%zu) of file '%s' is not an LPAREN."
            "The token at index %zu: %4zu:%-3zu:(%-19s) -> \"%.*s\".", LPAREN_index_candidate, tokens.file_path, LPAREN_index_candidate, LPAREN_token_candidate.location.line_num,
            LPAREN_token_candidate.location.col, al_token_kind_name(LPAREN_token_candidate.kind), (int)LPAREN_token_candidate.text_len, LPAREN_token_candidate.text);
        return FAIL;
    }
    size_t matching_RPAREN_index = 0, LPAREN_index = LPAREN_index_candidate;
    if (FAIL == LPAREN_find_matching_RPAREN(tokens, LPAREN_index, &matching_RPAREN_index, to_log)) {
        if (to_log) al_dprintERROR("Could not find matching RPAREN for the token at index %zu of file '%s'."
            "The token at index %zu: %4zu:%-3zu:(%-19s) -> \"%.*s\".",LPAREN_index, tokens.file_path, LPAREN_index,
            tokens.elements[LPAREN_index].location.line_num, tokens.elements[LPAREN_index].location.col,
            al_token_kind_name(tokens.elements[LPAREN_index].kind), (int)tokens.elements[LPAREN_index].text_len,
            tokens.elements[LPAREN_index].text);
        return FAIL;
    }
    size_t LBRACE_index_candidate = matching_RPAREN_index + 1;
    while (tokens.elements[LBRACE_index_candidate].kind == TOKEN_COMMENT) {
        LBRACE_index_candidate++;
        if (LBRACE_index_candidate >= tokens.length) {
            if (to_log) al_dprintERROR("%s", "Could not find LBRACE token.");
            return FAIL;
        }
    }
    size_t matching_RBRACE_index = 0, LBRACE_index = LBRACE_index_candidate;
    if (LBRACE_index >= tokens.length) {
        if (to_log) al_dprintERROR("LBRACE index (%zu) is bigger than tokens.length %zu.", LBRACE_index, tokens.length);
        return FAIL;
    }
    if (FAIL == LBRACE_find_matching_RBRACE(tokens, LBRACE_index, &matching_RBRACE_index, to_log)) {
        if (to_log) al_dprintERROR("Could not find matching RBRACE for the token at index %zu of file '%s'."
            "The token at index %zu: %4zu:%-3zu:(%-19s) -> \"%.*s\".", LBRACE_index, tokens.file_path, LBRACE_index,
            tokens.elements[LBRACE_index].location.line_num, tokens.elements[LBRACE_index].location.col,
            al_token_kind_name(tokens.elements[LBRACE_index].kind), (int)tokens.elements[LBRACE_index].text_len,
            tokens.elements[LBRACE_index].text);
        return FAIL;
    }

    if (func_def) {
        asm_strncpy(func_def->name, start_token.text, start_token.text_len);
        asm_strncpy(func_def->file_name, tokens.file_path, ASM_MAX_LEN);
        func_def->file_index = -1;
        func_def->token_start_index = start_index;
        func_def->token_end_index = matching_RBRACE_index;
        func_def->LPAREN_index = LPAREN_index;
        func_def->RPAREN_index = matching_RPAREN_index;
        func_def->LBRACE_index = LBRACE_index;
        func_def->RBRACE_index = matching_RBRACE_index;
    };

    return SUCCESS;
}

bool func_def_array_get_from_lexed_files_index(struct Lexed_Files lexed_files, size_t file_index, struct Func_Def_Array *func_def_array)
{
    if (file_index >= lexed_files.length) {
        al_dprintERROR("File index %zu is bigger the number of lexed files %zu", file_index, lexed_files.length);
        return FAIL;
    }
    struct Tokens tokens = lexed_files.elements[file_index];
    for (size_t i = 0; i < tokens.length; i++) {
        struct Function_Definition current_func = {0};
        if (SUCCESS == token_sequence_at_start_index_is_function_definition(tokens, i, &current_func, false)) {
            current_func.file_index = (int)file_index;
            if (func_def_array) {
                ada_appand(struct Function_Definition, *func_def_array, current_func);
                i = current_func.RBRACE_index + 1;
            }
        }
    }

    return SUCCESS;
}

bool lexed_files_get_function_definitions(struct Lexed_Files lexed_files, struct Func_Def_Array *func_def_array)
{
    for (size_t i = 0; i < lexed_files.length; i++) {
        if (FAIL == func_def_array_get_from_lexed_files_index(lexed_files, i, func_def_array)) {
            al_dprintERROR("Could not get function definition from lexed file at index %zu '%s'.", i, lexed_files.elements[i].file_path);
            return FAIL;
        }
    }

    return SUCCESS;
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        al_dprintERROR("Usage: %s 'entry_file.c' 'function_name'. Got %d arguments.", argv[0], argc-1);
        for (int i = 1; i < argc; i++) {
            printf("%*.sargv[%d] = [%s]\n", 8, "", i, argv[i]);
        }
        return -1;
    }
    
    const char *entry_file_relative_path = argv[1];
    const char *entry_function_name = argv[2];
    al_dprintINFO("entry file relative path = %s | entry function name = %s.", entry_file_relative_path, entry_function_name);
    if (APM_FAIL == apm_path_is_valid_file(entry_file_relative_path)) {
        al_dprintERROR("%s", "Entry file is not a valid file.");
        return -1;
    }

    Apm_Word current_working_directory;
    if (getcwd(current_working_directory, sizeof(current_working_directory)) == NULL) {
        al_dprintERROR("%s", "Could not get current working directory.");
        return -1;
    }

    Apm_Word entry_file_absolute_path;
    if (APM_SUCCESS == apm_path_is_absolute(entry_file_relative_path)) {
        asm_strncpy(entry_file_absolute_path, entry_file_relative_path, ASM_MAX_LEN);
    } else {
        if (APM_FAIL == apm_join_two_paths(entry_file_absolute_path, current_working_directory, (char *)entry_file_relative_path)) {
            al_dprintERROR("Could not join path2 '%s' to path1 '%s'.", (char *)entry_file_relative_path, current_working_directory);
            return -1;
        }
        if (APM_FAIL == apm_path_is_absolute(entry_file_absolute_path)) {
            al_dprintERROR("Could not create an absolute path to the entry file. Created: '%s'", entry_file_absolute_path);
            return -1;
        }
    }
    if (APM_FAIL == apm_path_fix(entry_file_absolute_path)) {
        al_dprintERROR("Could not fix path '%s'", entry_file_absolute_path);
        return -1;
    }
    asm_dprintSTRING(entry_file_absolute_path);

    printf("----------------------------------------\n");

    struct Lexed_Files lexed_files = {0};
    ada_init_array(struct Tokens, lexed_files);

    if (FAIL == lex_entire_file_recursively(&lexed_files, entry_file_absolute_path)) {
        al_dprintERROR("Could not lex recursively file '%s'.", entry_file_absolute_path);
        return -1;
    }
    al_dprintSIZE_T(lexed_files.length);
    for (size_t i = 0; i < lexed_files.length; i++) {
        printf("%*s%zu: %s\n", 7, "", i, lexed_files.elements[i].file_path);
    }

    printf("----------------------------------------\n");

    struct Func_Def_Array func_def_array = {0};
    ada_init_array(struct Function_Definition, func_def_array);
    if (FAIL == lexed_files_get_function_definitions(lexed_files, &func_def_array)) {
        al_dprintERROR("%s", "Could not get function definitions from lexes files.");
        return -1;
    }

    func_def_array_print(lexed_files, func_def_array);


    for (size_t i = 0; i <lexed_files.length; i++) {
        al_tokens_free(lexed_files.elements[i]);
    }
    AL_FREE(lexed_files.elements);
    AL_FREE(func_def_array.elements);
    return 0;
}
