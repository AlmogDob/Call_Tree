#include <stdio.h> 
 
// #define AMD_MEMORY_DEBUG
// #define ALMOG_MEMORY_DEBUG_IMPLEMENTATION
// #include "./includes/Almog_Memory_Debug.h"

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
    size_t start_line;
    size_t end_line;
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

struct Function_Call {
    char name[ASM_MAX_LEN];
    char file_name[ASM_MAX_LEN];
    int file_index;
    int called_at_func_def_index;
    int definition_func_def_index;
    size_t token_start_index;
    size_t token_end_index;
    size_t start_line;
    size_t end_line;
    size_t LPAREN_index;
    size_t RPAREN_index;
};

struct Func_Call_Array {
    size_t length;
    size_t capacity;
    struct Function_Call *elements;
};

struct Macro_Definition {
    char name[ASM_MAX_LEN];
    char file_name[ASM_MAX_LEN];
    int file_index;
    size_t pp_token_index;
    size_t start_line;
    size_t end_line;
};

struct Macro_Def_Array {
    size_t length;
    size_t capacity;
    struct Macro_Definition *elements;
};

struct Type_Declaration {
    size_t file_index;
    size_t token_start_index;
    size_t token_end_index;
};

struct Type_Declaration_Array {
    struct Type_Declaration *elements;
    size_t length;
    size_t capacity;
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
    printf("%*.sfile name         -> %s:%zu->%zu\n", (int)padding, "", func_def.file_name, func_def.start_line, func_def.end_line);
    printf("%*.sfile index        -> %-4d\n", (int)padding, "", func_def.file_index);
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

#define func_def_array_print(func_def_array) do {al_dprintINFO("%s = ", #func_def_array); func_def_array_print_imp(func_def_array, 11);} while (0)
void func_def_array_print_imp (struct Func_Def_Array func_def_array, size_t padding)
{
    for (size_t i = 0; i < func_def_array.length; i++) {
        struct Function_Definition func_def = func_def_array.elements[i];
        printf("%*.sFunc No.%zu:\n", (int)padding, "", i);
        printf("%*.s    name       -> %s\n", (int)padding, "", func_def.name);
        printf("%*.s    file name  -> %s:%zu->%zu\n", (int)padding, "", func_def.file_name, func_def.start_line, func_def.end_line);
        printf("%*.s    file index -> %-4d\n", (int)padding, "", func_def.file_index);
    }
}

#define function_call_print(tokens, func_call) do {al_dprintINFO("%s = ", #func_call); function_call_print_imp(tokens, func_call, 11);} while (0)
void function_call_print_imp(struct Tokens tokens, struct Function_Call func_call, size_t padding)
{
    printf("%*.sname                      -> %s\n", (int)padding, "", func_call.name);
    printf("%*.sfile name                 -> %s:%zu->%zu\n", (int)padding, "", func_call.file_name, func_call.start_line, func_call.end_line);
    printf("%*.sfile index                -> %-4d\n", (int)padding, "", func_call.file_index);
    printf("%*.scalled at func def index  -> %-4d\n", (int)padding, "", func_call.called_at_func_def_index);
    printf("%*.sdefinition func def index -> %-4d\n", (int)padding, "", func_call.definition_func_def_index);
    printf("%*.stoken start index         -> %-4zu = ", (int)padding, "", func_call.token_start_index);
    al_token_print(tokens.elements[func_call.token_start_index]);
    printf("%*.stoken end index           -> %-4zu = ", (int)padding, "", func_call.token_end_index);
    al_token_print(tokens.elements[func_call.token_end_index]);
    printf("%*.sLPAREN index              -> %-4zu = ", (int)padding, "", func_call.LPAREN_index);
    al_token_print(tokens.elements[func_call.LPAREN_index]);
    printf("%*.sRPAREN index              -> %-4zu = ", (int)padding, "", func_call.RPAREN_index);
    al_token_print(tokens.elements[func_call.RPAREN_index]);
}

#define func_call_array_print(func_call_array) do {al_dprintINFO("%s = ", #func_call_array); func_call_array_print_imp(func_call_array, 11);} while (0)
void func_call_array_print_imp (struct Func_Call_Array func_call_array, size_t padding)
{
    for (size_t i = 0; i < func_call_array.length; i++) {
        struct Function_Call func_call = func_call_array.elements[i];
        printf("%*.sFunc No.%zu:\n", (int)padding, "", i);
        printf("%*.s    name                      -> %s\n", (int)padding, "", func_call.name);
        printf("%*.s    file name                 -> %s:%zu->%zu\n", (int)padding, "", func_call.file_name, func_call.start_line, func_call.end_line);
        printf("%*.s    file index                -> %-4d\n", (int)padding, "", func_call.file_index);
        printf("%*.s    called at func def index  -> %-4d\n", (int)padding, "", func_call.called_at_func_def_index);
        printf("%*.s    definition func def index -> %-4d\n", (int)padding, "", func_call.definition_func_def_index);
    }
}

void pp_directive_splice_lines(char *s)
{
    size_t r = 0;
    size_t w = 0;

    while (s[r] != '\0') {
        if (s[r] == '\\' && s[r + 1] == '\n') {
            r += 2;
            continue;
        }

        if (s[r] == '\\' && s[r + 1] == '\r' && s[r + 2] == '\n') {
            r += 3;
            continue;
        }

        s[w++] = s[r++];
    }

    s[w] = '\0';
}

bool include_paths_get_from_tokens(struct Apm_Word_Array *word_array, struct Tokens tokens)
{
    Apm_Word temp_word;
    Apm_Word current_directive;
    for (size_t i = 0; i < tokens.length; i++) {
        struct Token token = tokens.elements[i];
        if (token.kind == TOKEN_PP_DIRECTIVE) {
            asm_strncpy(current_directive, token.text, token.text_len);
            pp_directive_splice_lines(current_directive);
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
    if (APM_FAIL == apm_paths_add_prefix(*include_paths, entry_file_absolute_dir)) {
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

size_t function_definition_real_start_index_get(struct Tokens tokens, size_t name_index)
{
    size_t i = name_index;

    while (i > 0) {
        enum Token_Kind prev_kind = tokens.elements[i - 1].kind;

        if (prev_kind == TOKEN_SEMICOLON ||
            prev_kind == TOKEN_LBRACE ||
            prev_kind == TOKEN_RBRACE ||
            prev_kind == TOKEN_PP_DIRECTIVE) {
            break;
        }

        i--;
    }

    while (i < name_index && tokens.elements[i].kind == TOKEN_COMMENT) {
        i++;
    }

    return i;
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
    // if (FAIL == LBRACE_find_matching_RBRACE(tokens, LBRACE_index, &matching_RBRACE_index, true)) {
    //     al_dprintERROR("Could not find matching RBRACE for the token at index %zu of file '%s'."
            "The token at index %zu: %4zu:%-3zu:(%-19s) -> \"%.*s\".", LBRACE_index, tokens.file_path, LBRACE_index,
            tokens.elements[LBRACE_index].location.line_num, tokens.elements[LBRACE_index].location.col,
            al_token_kind_name(tokens.elements[LBRACE_index].kind), (int)tokens.elements[LBRACE_index].text_len,
            tokens.elements[LBRACE_index].text);
        return FAIL;
    }

    if (func_def) {
        size_t real_start_index = function_definition_real_start_index_get(tokens, start_index);
        asm_strncpy(func_def->name, start_token.text, start_token.text_len);
        asm_strncpy(func_def->file_name, tokens.file_path, ASM_MAX_LEN);
        func_def->file_index = -1;
        func_def->token_start_index = real_start_index;
        func_def->token_end_index = matching_RBRACE_index;
        func_def->start_line = tokens.elements[func_def->token_start_index].location.line_num;
        func_def->end_line = tokens.elements[func_def->token_end_index].location.line_num;
        func_def->LPAREN_index = LPAREN_index;
        func_def->RPAREN_index = matching_RPAREN_index;
        func_def->LBRACE_index = LBRACE_index;
        func_def->RBRACE_index = matching_RBRACE_index;
    }

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
                i = current_func.RBRACE_index;
            }
        }
    }

    return SUCCESS;
}

bool function_definitions_get_from_lexed_files(struct Lexed_Files lexed_files, struct Func_Def_Array *func_def_array)
{
    for (size_t i = 0; i < lexed_files.length; i++) {
        if (FAIL == func_def_array_get_from_lexed_files_index(lexed_files, i, func_def_array)) {
            al_dprintERROR("Could not get function definition from lexed file at index %zu '%s'.", i, lexed_files.elements[i].file_path);
            return FAIL;
        }
    }

    return SUCCESS;
}

bool token_sequence_at_start_index_is_function_call(struct Tokens tokens, size_t start_index, struct Function_Call *func_call, bool to_log) {
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

    if (func_call) {
        asm_strncpy(func_call->name, start_token.text, start_token.text_len);
        asm_strncpy(func_call->file_name, tokens.file_path, ASM_MAX_LEN);
        func_call->file_index = -1;
        func_call->called_at_func_def_index = -1;
        func_call->definition_func_def_index = -1;
        func_call->token_start_index = start_index;
        func_call->token_end_index = matching_RPAREN_index;
        func_call->start_line = tokens.elements[func_call->token_start_index].location.line_num;
        func_call->end_line = tokens.elements[func_call->token_end_index].location.line_num;
        func_call->LPAREN_index = LPAREN_index;
        func_call->RPAREN_index = matching_RPAREN_index;
    };

    return SUCCESS;
}

int func_def_index_get_by_name(struct Func_Def_Array func_def_array, const char *func_name)
{
    for (size_t i = 0; i < func_def_array.length; i++) {
        if (asm_strncmp(func_def_array.elements[i].name, func_name, ASM_MAX_LEN)) {
            return (int)i;
        }
    }

    return -1;
}

int func_call_set_func_def_index(struct Func_Def_Array func_def_array, struct Function_Call *func_call)
{
    for (size_t i = 0; i < func_def_array.length; i++) {
        if (asm_strncmp(func_call->name, func_def_array.elements[i].name, ASM_MAX_LEN)) {
            func_call->definition_func_def_index = (int)i;
            return (int)i;
        }
    }

    return -1;
}

void func_call_array_set_func_def_index(struct Func_Def_Array func_def_array, struct Func_Call_Array func_call_array)
{
    for (size_t i = 0; i < func_call_array.length; i++) {
        func_call_set_func_def_index(func_def_array, &func_call_array.elements[i]);
    }
}

bool func_call_array_get_from_lexed_files_index(struct Lexed_Files lexed_files, size_t file_index, struct Func_Call_Array *func_call_array)
{
    if (file_index >= lexed_files.length) {
        al_dprintERROR("File index %zu is bigger the number of lexed files %zu", file_index, lexed_files.length);
        return FAIL;
    }
    struct Tokens tokens = lexed_files.elements[file_index];
    for (size_t i = 0; i < tokens.length; i++) {
        struct Function_Call current_func_call = {0};
        if (SUCCESS == token_sequence_at_start_index_is_function_call(tokens, i, &current_func_call, false)) {
            current_func_call.file_index = (int)file_index;
            if (func_call_array) {
                ada_appand(struct Function_Call, *func_call_array, current_func_call);
            }
        }
    }

    return SUCCESS;
}

bool func_call_array_get_from_func_def_index(struct Lexed_Files lexed_files, struct Func_Def_Array func_def_array, size_t func_def_index, struct Func_Call_Array *func_call_array)
{
    if (func_def_index >= func_def_array.length) {
        al_dprintERROR("Function definition index %zu is bigger the number of function definitions %zu", func_def_index, func_def_array.length);
        return FAIL;
    }

    struct Function_Definition func_def = func_def_array.elements[func_def_index];
    struct Tokens current_lexed_file = lexed_files.elements[func_def.file_index];
    for (size_t token_index = func_def.LBRACE_index + 1; token_index < func_def.RBRACE_index; token_index++) {
        struct Function_Call current_func_call = {0};
        if (SUCCESS == token_sequence_at_start_index_is_function_call(current_lexed_file, token_index, &current_func_call, false)) {
            current_func_call.file_index = (int)func_def.file_index;
            current_func_call.called_at_func_def_index = (int)func_def_index;
            func_call_set_func_def_index(func_def_array, &current_func_call);
            if (func_call_array) {
                ada_appand(struct Function_Call, *func_call_array, current_func_call);
            }
        }
    }

    return SUCCESS;
}

bool func_call_array_get_from_func_def_array(struct Lexed_Files lexed_files, struct Func_Def_Array func_def_array, struct Func_Call_Array *func_call_array)
{
    for (size_t i = 0; i < func_def_array.length;i ++) {
        if (FAIL == func_call_array_get_from_func_def_index(lexed_files, func_def_array, i, func_call_array)) {
            al_dprintERROR("Could not get function call array from function definition at index %zu '%s', at file '%s'", i, func_def_array.elements[i].name, lexed_files.elements[func_def_array.elements[i].file_index].file_path);
            return FAIL;
        }
    }

    return SUCCESS;
}

bool func_def_in_func_def_array(struct Func_Def_Array func_def_array, struct Function_Definition func_def)
{
    for (size_t i = 0; i < func_def_array.length; i++) {
        if (asm_strncmp(func_def.name, func_def_array.elements[i].name, ASM_MAX_LEN)) {
            return SUCCESS;
        }
    }

    return FAIL;
}

static bool func_defs_to_print_visit(struct Func_Def_Array func_def_array, struct Func_Call_Array func_call_array, size_t func_def_index, unsigned char *state, struct Func_Def_Array *func_defs_to_print)
{
    /* by AI */

    /**
     *  state = 0 -> not visited 
     *  state = 1 -> currently being visited 
     *  state = 2 -> finished and added to the output
     */
    if (state[func_def_index] == 2) {
        return SUCCESS;
    }

    if (state[func_def_index] == 1) {
        /*
        * Already on the current traversal path.
        * Forward declarations will handle this dependency.
        */
        return SUCCESS;
    }

    state[func_def_index] = 1;

    for (size_t i = 0; i < func_call_array.length; i++) {
        struct Function_Call call = func_call_array.elements[i];

        if (call.called_at_func_def_index < 0 || (size_t)call.called_at_func_def_index != func_def_index ||
            call.definition_func_def_index < 0) {
            continue;
        }

        size_t callee_index = (size_t)call.definition_func_def_index;

        if (callee_index >= func_def_array.length) {
            al_dprintERROR("%s", "Invalid callee definition index.");
            return FAIL;
        }

        /*
         * Direct self-recursion is valid in C: the function's name is
         * already declared within its own body.
         */
        if (callee_index == func_def_index) {
            continue;
        }

        if (FAIL == func_defs_to_print_visit(func_def_array, func_call_array, callee_index, state, func_defs_to_print)) {
            return FAIL;
        }
    }

    /* Append only AFTER all callees have been appended. */
    ada_appand(struct Function_Definition, *func_defs_to_print, func_def_array.elements[func_def_index]);

    state[func_def_index] = 2;
    return SUCCESS;
}

bool func_defs_to_print_get_from_func_def_index(struct Func_Def_Array func_def_array, struct Func_Call_Array func_call_array, size_t entry_func_def_index, struct Func_Def_Array *func_defs_to_print)
{
    if (func_defs_to_print == NULL || entry_func_def_index >= func_def_array.length) {
        return FAIL;
    }

    unsigned char *state = AL_MALLOC(func_def_array.length * sizeof(*state));
    if (state == NULL) {
        al_dprintERROR("%s", "Could not allocate traversal state.");
        return FAIL;
    }

    for (size_t i = 0; i < func_def_array.length; i++) {
        state[i] = 0;
    }

    size_t original_length = func_defs_to_print->length;

    bool result = func_defs_to_print_visit(func_def_array, func_call_array, entry_func_def_index, state, func_defs_to_print);

    if (result == FAIL) {
        func_defs_to_print->length = original_length;
    }

    AL_FREE(state);
    return result;
}

void func_def_array_content_print_to_output_target(FILE *output_target, struct Func_Def_Array func_def_array, struct Lexed_Files lexed_files)
{
    for (size_t i = 0; i < func_def_array.length; i++) {
        size_t current_func_def_index = i;
        struct Function_Definition current_func_def = func_def_array.elements[current_func_def_index];
        struct Tokens tokens = lexed_files.elements[current_func_def.file_index];
        const char *start_char_pointer = tokens.elements[current_func_def.token_start_index].text;
        const char *end_char_pointer = tokens.elements[current_func_def.token_end_index].text + tokens.elements[current_func_def.token_end_index].text_len - 1;
        for (char *cp = (char *)start_char_pointer; cp != end_char_pointer; cp++) {
            fputc(*cp, output_target);
        }
        fputc(*end_char_pointer, output_target);
        fputc('\n', output_target);
        fputc('\n', output_target);
    }
}

bool pp_directive_get_define_name(char *name_out, struct Token pp_token, bool to_log)
{
    if (pp_token.kind != TOKEN_PP_DIRECTIVE) {
        if (to_log) {
            al_dprintERROR("%s", "Input token is not a preprocessor directive.");
        }
        return FAIL;
    }
    if (pp_token.text_len >= ASM_MAX_LEN) {
        if (to_log) {
            al_dprintERROR("Preprocessor directive is too long (%zu >= %d).", pp_token.text_len, ASM_MAX_LEN);
        }
        return FAIL;
    }

    char line[ASM_MAX_LEN];
    asm_strncpy(line, pp_token.text, pp_token.text_len);
    line[pp_token.text_len] = '\0';
    pp_directive_splice_lines(line);

    size_t i = 0;
    while (asm_isspace(line[i])) {
        i++;
    }
    if (line[i++] != '#') {
        return FAIL;
    }
    while (asm_isspace(line[i])) {
        i++;
    }
    if (!asm_strncmp(line + i, "define", 6)) {
        return FAIL;
    }
    i += 6;

    if (!asm_isspace(line[i])) {
        /* Reject things like "#defined" */
        return FAIL;
    }
    while (asm_isspace(line[i])) {
        i++;
    }

    if (!al_is_identifier_start(line[i])) {
        if (to_log) {
            al_dprintERROR("Could not parse macro name from directive: \"%s\"", line);
        }
        return FAIL;
    }

    size_t j = 0;
    while (al_is_identifier(line[i])) {
        if (j + 1 >= ASM_MAX_LEN) {
            if (to_log) {
                al_dprintERROR("%s", "Macro name is too long.");
            }
            return FAIL;
        }
        name_out[j++] = line[i++];
    }
    name_out[j] = '\0';

    return SUCCESS;
}

bool token_sequence_at_index_is_macro_definition(struct Tokens tokens, size_t token_index, struct Macro_Definition *macro_def, bool to_log)
{
    if (token_index >= tokens.length) {
        if (to_log) {
            al_dprintERROR("Token index %zu is out of range. tokens.length = %zu", token_index, tokens.length);
        }
        return FAIL;
    }

    struct Token token = tokens.elements[token_index];
    char macro_name[ASM_MAX_LEN];

    if (FAIL == pp_directive_get_define_name(macro_name, token, to_log)) {
        if (to_log) {
            al_dprintERROR("%s", "Could not get macro name form PP directive.");
        }
        return FAIL;
    }

    if (macro_def) {
        asm_strncpy(macro_def->name, macro_name, ASM_MAX_LEN);
        asm_strncpy(macro_def->file_name, tokens.file_path, ASM_MAX_LEN);
        macro_def->file_index = -1;
        macro_def->pp_token_index = token_index;
        macro_def->start_line = token.location.line_num;
        macro_def->end_line = token.location.line_num;
    }

    return SUCCESS;
}

bool macro_def_array_get_from_lexed_files_index(struct Lexed_Files lexed_files, size_t file_index, struct Macro_Def_Array *macro_def_array)
{
    if (file_index >= lexed_files.length) {
        al_dprintERROR("File index %zu is bigger than lexed_files.length %zu", file_index, lexed_files.length);
        return FAIL;
    }

    struct Tokens tokens = lexed_files.elements[file_index];

    for (size_t i = 0; i < tokens.length; i++) {
        struct Macro_Definition current_macro = {0};

        if (SUCCESS == token_sequence_at_index_is_macro_definition(tokens, i, &current_macro, false)) {
            current_macro.file_index = (int)file_index;
            ada_appand(struct Macro_Definition, *macro_def_array, current_macro);
        }
    }

    return SUCCESS;
}

bool macro_definitions_get_from_lexed_files(struct Lexed_Files lexed_files, struct Macro_Def_Array *macro_def_array)
{
    for (size_t i = 0; i < lexed_files.length; i++) {
        if (FAIL == macro_def_array_get_from_lexed_files_index(lexed_files, i, macro_def_array)) {
            al_dprintERROR("Could not get macro definitions from file index %zu '%s'.", i, lexed_files.elements[i].file_path);
            return FAIL;
        }
    }

    return SUCCESS;
}

int macro_def_index_get_by_name(struct Macro_Def_Array macro_def_array, const char *macro_name)
{
    for (size_t i = 0; i < macro_def_array.length; i++) {
        if (asm_strncmp(macro_def_array.elements[i].name, macro_name, ASM_MAX_LEN)) {
            return (int)i;
        }
    }

    return -1;
}

bool macro_def_in_macro_def_array(struct Macro_Def_Array macro_def_array, struct Macro_Definition macro_def)
{
    for (size_t i = 0; i < macro_def_array.length; i++) {
        if (asm_strncmp(macro_def_array.elements[i].name, macro_def.name, ASM_MAX_LEN)) {
            return SUCCESS;
        }
    }

    return FAIL;
}

void macro_defs_used_in_func_def_get(struct Lexed_Files lexed_files, struct Function_Definition func_def, struct Macro_Def_Array all_macro_defs, struct Macro_Def_Array *used_macro_defs)
{
    /* By AI */
    struct Tokens tokens = lexed_files.elements[func_def.file_index];

    for (size_t i = func_def.token_start_index; i <= func_def.token_end_index; i++) {
        struct Token token = tokens.elements[i];
        if (token.kind != TOKEN_IDENTIFIER) {
            continue;
        }

        for (size_t j = 0; j < all_macro_defs.length; j++) {
            struct Macro_Definition macro_def = all_macro_defs.elements[j];
            if (al_token_text_equals_str(token, macro_def.name)) {
                if (FAIL == macro_def_in_macro_def_array(*used_macro_defs, macro_def)) {
                    ada_appand(struct Macro_Definition, *used_macro_defs, macro_def);
                }
                break;
            }
        }
    }
}

void macro_defs_used_in_func_def_array_get(struct Lexed_Files lexed_files, struct Func_Def_Array func_defs_to_print, struct Macro_Def_Array all_macro_defs, struct Macro_Def_Array *used_macro_defs)
{
    /* By AI */
    for (size_t i = 0; i < func_defs_to_print.length; i++) {
        macro_defs_used_in_func_def_get(lexed_files, func_defs_to_print.elements[i], all_macro_defs, used_macro_defs);
    }
}

void macro_def_array_content_print_to_output_target(FILE *output_target, struct Macro_Def_Array macro_def_array, struct Lexed_Files lexed_files)
{
    /* By AI */
    for (size_t i = 0; i < macro_def_array.length; i++) {
        struct Macro_Definition macro_def = macro_def_array.elements[i];
        struct Tokens tokens = lexed_files.elements[macro_def.file_index];
        struct Token pp_token = tokens.elements[macro_def.pp_token_index];

        fwrite(pp_token.text, 1, pp_token.text_len, output_target);

        if (pp_token.text_len == 0 ||
            pp_token.text[pp_token.text_len - 1] != '\n') {
            fputc('\n', output_target);
        }

        fputc('\n', output_target);
    }
}

static size_t macro_text_skip_trivia(const char *text, size_t i)
{
    /* By AI */
    for (;;) {
        while (asm_isspace(text[i])) {
            i++;
        }

        if (text[i] == '/' && text[i + 1] == '*') {
            i += 2;

            while (text[i] != '\0' &&
                   !(text[i] == '*' && text[i + 1] == '/')) {
                i++;
            }

            if (text[i] != '\0') {
                i += 2;
            }

            continue;
        }

        return i;
    }
}

static void macro_function_call_record(const char *name, const char *text, size_t after_name, struct Macro_Definition macro_def, struct Func_Def_Array func_def_array, int caller_index, struct Func_Call_Array *func_call_array)
{
    /* By AI */
    /*
     * A negative caller means we are only collecting macro dependencies,
     * not adding edges to the function-call graph.
     */
    if (caller_index < 0) {
        return;
    }

    size_t next = macro_text_skip_trivia(text, after_name);

    if (text[next] != '(') {
        return;
    }

    int callee_index = func_def_index_get_by_name(func_def_array, name);

    if (callee_index < 0) {
        /* No definition was found in the lexed files. */
        return;
    }

    /* Only one graph edge per caller/callee pair is needed. */
    for (size_t i = 0; i < func_call_array->length; i++) {
        struct Function_Call call = func_call_array->elements[i];

        if (call.called_at_func_def_index == caller_index &&
            call.definition_func_def_index == callee_index) {
            return;
        }
    }

    struct Function_Call call = {0};

    asm_strncpy(call.name, name, ASM_MAX_LEN);
    asm_strncpy(call.file_name, macro_def.file_name, ASM_MAX_LEN);

    call.file_index = macro_def.file_index;
    call.called_at_func_def_index = caller_index;
    call.definition_func_def_index = callee_index;
    call.start_line = macro_def.start_line;
    call.end_line = macro_def.end_line;

    /*
     * Synthetic graph edge, not a normal tokenized function call.
     * These indexes must not be used by function_call_print_imp().
     */
    call.token_start_index = SIZE_MAX;
    call.token_end_index = SIZE_MAX;
    call.LPAREN_index = SIZE_MAX;
    call.RPAREN_index = SIZE_MAX;

    ada_appand(struct Function_Call, *func_call_array, call);
}

void macro_defs_used_in_macro_def_get(struct Lexed_Files lexed_files, struct Macro_Definition macro_def, struct Macro_Def_Array all_macro_defs, struct Macro_Def_Array *used_macro_defs, struct Func_Def_Array func_def_array, int caller_index, struct Func_Call_Array *func_call_array)
{
    /* By AI */
    struct Tokens tokens = lexed_files.elements[macro_def.file_index];
    struct Token token = tokens.elements[macro_def.pp_token_index];

    if (token.text_len >= ASM_MAX_LEN) {
        al_dprintWARNING("Skipping dependencies of macro '%s': directive is too long.", macro_def.name);
        return;
    }

    char line[ASM_MAX_LEN];
    asm_strncpy(line, token.text, token.text_len);
    line[token.text_len] = '\0';
    pp_directive_splice_lines(line);

    size_t i = 0;

    /* Skip whitespace, '#', whitespace, 'define', and whitespace. */
    while (asm_isspace(line[i])) {
        i++;
    }
    if (line[i] != '#') {
        return;
    }
    i++;
    while (asm_isspace(line[i])) {
        i++;
    }
    if (!asm_strncmp(line + i, "define", 6)) {
        return;
    }
    i += 6;
    while (asm_isspace(line[i])) {
        i++;
    }

    /* Skip the macro's own name. */
    while (al_is_identifier(line[i])) {
        i++;
    }

    /*
     * For function-like macros, '(' immediately follows the name.
     * Skip the formal parameter list.
     */
    size_t parameters_start = i;
    size_t parameters_end = i;

    if (line[i] == '(') {
        i++;
        while (line[i] != '\0' && line[i] != ')') {
            i++;
        }
        if (line[i] != ')') {
            return;
        }
        i++;
        parameters_end = i;
    }

    while (line[i] != '\0') {
        /* Skip string and character literals. */
        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i++];

            while (line[i] != '\0') {
                if (line[i] == '\\' && line[i + 1] != '\0') {
                    i += 2;
                } else if (line[i++] == quote) {
                    break;
                }
            }
            continue;
        }

        /* Skip block comments. */
        if (line[i] == '/' && line[i + 1] == '*') {
            i += 2;

            while (line[i] != '\0' &&
                   !(line[i] == '*' && line[i + 1] == '/')) {
                i++;
            }

            if (line[i] != '\0') {
                i += 2;
            }
            continue;
        }

        /* A line comment consumes the rest of this directive. */
        if (line[i] == '/' && line[i + 1] == '/') {
            break;
        }

        if (!al_is_identifier_start(line[i])) {
            i++;
            continue;
        }

        char name[ASM_MAX_LEN];
        size_t name_length = 0;

        while (al_is_identifier(line[i])) {
            name[name_length++] = line[i++];
        }
        name[name_length] = '\0';

        /*
         * Formal parameters are not references to global macros,
         * even when a macro with the same name exists.
         */
        bool is_parameter = false;

        for (size_t p = parameters_start; p < parameters_end;) {
            if (!al_is_identifier_start(line[p])) {
                p++;
                continue;
            }

            size_t start = p;

            while (p < parameters_end && al_is_identifier(line[p])) {
                p++;
            }

            if (p - start == name_length &&
                asm_strncmp(line + start, name, name_length)) {
                is_parameter = true;
                break;
            }
        }

        if (is_parameter) {
            continue;
        }

        int index = macro_def_index_get_by_name(all_macro_defs, name);
        if (index >= 0) {
            struct Macro_Definition dependency = all_macro_defs.elements[index];
            if (FAIL == macro_def_in_macro_def_array(*used_macro_defs, dependency)) {
                ada_appand(struct Macro_Definition, *used_macro_defs, dependency);
            }
        } else {
            /*
            * This identifier is not a known macro. If it is followed by '('
            * and names a known function, add a function-call graph edge.
            */
            macro_function_call_record(name, line, i, macro_def, func_def_array, caller_index, func_call_array);
        }
    }
}

void macro_defs_dependencies_get(struct Lexed_Files lexed_files, struct Macro_Def_Array all_macro_defs, struct Macro_Def_Array *used_macro_defs, struct Func_Def_Array func_def_array, int caller_index, struct Func_Call_Array *func_call_array)
{
    /* By AI */
    for (size_t i = 0; i < used_macro_defs->length; i++) {
        struct Macro_Definition macro_def = used_macro_defs->elements[i];

        macro_defs_used_in_macro_def_get(lexed_files, macro_def, all_macro_defs, used_macro_defs, func_def_array, caller_index, func_call_array);
    }
}

void func_call_array_add_macro_calls(struct Lexed_Files lexed_files, struct Func_Def_Array func_def_array, struct Macro_Def_Array all_macro_defs, struct Func_Call_Array *func_call_array)
{
    /* By AI */
    for (size_t i = 0; i < func_def_array.length; i++) {
        struct Macro_Def_Array used_macros = {0};
        ada_init_array(struct Macro_Definition, used_macros);

        macro_defs_used_in_func_def_get(lexed_files, func_def_array.elements[i], all_macro_defs, &used_macros);
        macro_defs_dependencies_get(lexed_files, all_macro_defs, &used_macros, func_def_array, (int)i, func_call_array);

        AL_FREE(used_macros.elements);
    }
}

static size_t type_next_nontrivia(struct Tokens tokens, size_t index)
{
    /* By AI */
    while (index < tokens.length) {
        enum Token_Kind kind = tokens.elements[index].kind;
        if (kind != TOKEN_COMMENT && kind != TOKEN_PP_DIRECTIVE) {
            break;
        }
        index++;
    }

    return index;
}

static bool token_starts_type_declaration(struct Tokens tokens, size_t index)
{
    /* By AI */
    if (index >= tokens.length) {
        return false;
    }

    struct Token token = tokens.elements[index];

    if (al_token_text_equals_str(token, "typedef")) {
        return true;
    }

    bool is_tag =
        al_token_text_equals_str(token, "struct") ||
        al_token_text_equals_str(token, "union") ||
        al_token_text_equals_str(token, "enum");

    if (!is_tag) {
        return false;
    }

    size_t next = type_next_nontrivia(tokens, index + 1);

    /* Optional struct/union/enum tag name. */
    if (next < tokens.length && tokens.elements[next].kind == TOKEN_IDENTIFIER) {
        next = type_next_nontrivia(tokens, next + 1);
    }

    if (next >= tokens.length) {
        return false;
    }

    enum Token_Kind kind = tokens.elements[next].kind;

    return kind == TOKEN_LBRACE || kind == TOKEN_SEMICOLON;
}

static bool declaration_end_get(struct Tokens tokens, size_t start, size_t *end_out)
{
    /* By AI */
    size_t braces = 0;
    size_t parentheses = 0;
    size_t brackets = 0;

    for (size_t i = start; i < tokens.length; i++) {
        enum Token_Kind kind = tokens.elements[i].kind;

        switch (kind) {
            case TOKEN_LBRACE:
                braces++;
                break;

            case TOKEN_RBRACE:
                if (braces == 0) {
                    return false;
                }
                braces--;
                break;

            case TOKEN_LPAREN:
                parentheses++;
                break;

            case TOKEN_RPAREN:
                if (parentheses == 0) {
                    return false;
                }
                parentheses--;
                break;

            case TOKEN_LBRACKET:
                brackets++;
                break;

            case TOKEN_RBRACKET:
                if (brackets == 0) {
                    return false;
                }
                brackets--;
                break;

            case TOKEN_SEMICOLON:
                if (braces == 0 && parentheses == 0 && brackets == 0) {
                    *end_out = i;
                    return true;
                }
                break;

            case TOKEN_EOF:
                return false;

            default:
                break;
        }
    }

    return false;
}

static bool function_range_end_get(struct Func_Def_Array functions, size_t file_index, size_t token_index, size_t *end_out)
{
    /* By AI */
    for (size_t i = 0; i < functions.length; i++) {
        struct Function_Definition function = functions.elements[i];

        if (function.file_index < 0 || (size_t)function.file_index != file_index) {
            continue;
        }

        if (token_index >= function.token_start_index && token_index <= function.token_end_index) {
            *end_out = function.token_end_index;
            return true;
        }
    }

    return false;
}

bool type_declarations_get_from_lexed_files(struct Lexed_Files files, struct Func_Def_Array functions, struct Type_Declaration_Array *declarations)
{
    /* By AI */
    if (declarations == NULL) {
        return FAIL;
    }

    for (size_t file_index = 0; file_index < files.length;
         file_index++) {
        struct Tokens tokens = files.elements[file_index];
        size_t i = 0;

        while (i < tokens.length) {
            i = type_next_nontrivia(tokens, i);

            if (i >= tokens.length || tokens.elements[i].kind == TOKEN_EOF) {
                break;
            }

            size_t end = 0;

            if (function_range_end_get(functions, file_index, i, &end)) {
                i = end + 1;
                continue;
            }

            bool is_type = token_starts_type_declaration(tokens, i);

            if (!declaration_end_get(tokens, i, &end)) {
                al_dprintERROR("Could not find declaration end in '%s' " "at line %zu.", tokens.file_path, tokens.elements[i].location.line_num);
                return FAIL;
            }

            if (is_type) {
                struct Type_Declaration declaration = {
                    .file_index = file_index,
                    .token_start_index = i,
                    .token_end_index = end,
                };

                ada_appand(struct Type_Declaration, *declarations, declaration);
            }

            i = end + 1;
        }
    }

    return SUCCESS;
}

void type_declarations_print_to_output_target(FILE *output_target, struct Type_Declaration_Array declarations, struct Lexed_Files files)
{
    /* By AI */
    for (size_t i = 0; i < declarations.length; i++) {
        struct Type_Declaration declaration = declarations.elements[i];
        struct Tokens tokens = files.elements[declaration.file_index];
        struct Token first = tokens.elements[declaration.token_start_index];
        struct Token last = tokens.elements[declaration.token_end_index];
        const char *begin = first.text;
        const char *end = last.text + last.text_len;

        fwrite(begin, 1, (size_t)(end - begin), output_target);
        fputs("\n\n", output_target);
    }
}

static bool type_token_is( struct Tokens tokens, size_t index, const char *text)
{
    /* By AI */
    return index < tokens.length && al_token_text_equals_str(tokens.elements[index], text);
}

static bool type_skip_balanced(struct Tokens tokens, size_t *index, size_t end, enum Token_Kind open, enum Token_Kind close)
{
    /* By AI */
    if (*index >= end || tokens.elements[*index].kind != open) {
        return false;
    }

    size_t depth = 0;

    while (*index < end) {
        enum Token_Kind kind = tokens.elements[*index].kind;
        (*index)++;

        if (kind == open) {
            depth++;
        } else if (kind == close) {
            depth--;

            if (depth == 0) {
                return true;
            }
        }
    }

    return false;
}

static bool type_is_pointer_qualifier(struct Tokens tokens, size_t index)
{
    /* By AI */
    return type_token_is(tokens, index, "const") ||
           type_token_is(tokens, index, "volatile") ||
           type_token_is(tokens, index, "restrict") ||
           type_token_is(tokens, index, "_Atomic");
}

/*
 * Parse a declarator and return the token containing its declared name.
 *
 * Examples:
 *     Name
 *     *Name
 *     Name[10]
 *     (*Name)(int)
 */
static bool type_declarator_name_get(struct Tokens tokens, size_t *index, size_t end, size_t *name_index)
{
    /* By AI */
    *index = type_next_nontrivia(tokens, *index);

    while (*index < end && tokens.elements[*index].kind == TOKEN_STAR) {
        (*index)++;
        *index = type_next_nontrivia(tokens, *index);

        while (*index < end && type_is_pointer_qualifier(tokens, *index)) {
            (*index)++;
            *index = type_next_nontrivia(tokens, *index);
        }
    }

    if (*index >= end) {
        return false;
    }

    if (tokens.elements[*index].kind == TOKEN_IDENTIFIER) {
        *name_index = *index;
        (*index)++;
    } else if (tokens.elements[*index].kind == TOKEN_LPAREN) {
        (*index)++;

        if (!type_declarator_name_get(tokens, index, end, name_index)) {
            return false;
        }

        *index = type_next_nontrivia(tokens, *index);

        if (*index >= end ||
            tokens.elements[*index].kind != TOKEN_RPAREN) {
            return false;
        }

        (*index)++;
    } else {
        return false;
    }

    for (;;) {
        *index = type_next_nontrivia(tokens, *index);

        if (*index >= end) {
            return true;
        }

        enum Token_Kind kind = tokens.elements[*index].kind;

        if (kind == TOKEN_LBRACKET) {
            if (!type_skip_balanced(tokens, index, end, TOKEN_LBRACKET, TOKEN_RBRACKET)) {
                return false;
            }
        } else if (kind == TOKEN_LPAREN) {
            if (!type_skip_balanced(tokens, index, end, TOKEN_LPAREN, TOKEN_RPAREN)) {
                return false;
            }
        } else {
            return true;
        }
    }
}

static bool typedef_declarators_start_get(struct Tokens tokens, struct Type_Declaration declaration, size_t *start_out)
{
    /* By AI */
    size_t i = declaration.token_start_index;
    size_t end = declaration.token_end_index;

    if (!type_token_is(tokens, i, "typedef")) {
        return false;
    }

    i++;
    bool have_type = false;

    while (i < end) {
        i = type_next_nontrivia(tokens, i);

        if (i >= end) {
            return false;
        }

        if (type_token_is(tokens, i, "struct") ||
            type_token_is(tokens, i, "union") ||
            type_token_is(tokens, i, "enum")) {
            i++;
            i = type_next_nontrivia(tokens, i);

            if (i < end &&
                tokens.elements[i].kind == TOKEN_IDENTIFIER) {
                i++;
                i = type_next_nontrivia(tokens, i);
            }

            if (i < end &&
                tokens.elements[i].kind == TOKEN_LBRACE) {
                if (!type_skip_balanced(tokens, &i, end, TOKEN_LBRACE, TOKEN_RBRACE)) {
                    return false;
                }
            }

            have_type = true;
            continue;
        }

        if (type_is_pointer_qualifier(tokens, i)) {
            /*
             * Handle the type-specifier form _Atomic(T).
             * Plain _Atomic is a qualifier.
             */
            bool atomic = type_token_is(tokens, i, "_Atomic");
            i++;
            i = type_next_nontrivia(tokens, i);

            if (atomic && i < end &&
                tokens.elements[i].kind == TOKEN_LPAREN) {
                if (!type_skip_balanced(tokens, &i, end, TOKEN_LPAREN, TOKEN_RPAREN)) {
                    return false;
                }

                have_type = true;
            }

            continue;
        }

        /*
         * Ordinary built-in type specifiers, such as unsigned,
         * long, int, void, and double, are keyword tokens.
         */
        if (tokens.elements[i].kind == TOKEN_KEYWORD) {
            have_type = true;
            i++;
            continue;
        }

        /*
         * A typedef name used as the underlying type:
         *
         *     typedef Existing_Type New_Type;
         */
        if (!have_type && tokens.elements[i].kind == TOKEN_IDENTIFIER) {
            have_type = true;
            i++;
            continue;
        }

        break;
    }

    *start_out = i;
    return have_type && i < end;
}

static bool type_declaration_provides_name(struct Tokens tokens, struct Type_Declaration declaration, struct Token reference)
{
    /* By AI */
    size_t start = declaration.token_start_index;
    size_t end = declaration.token_end_index;
    size_t braces = 0;

    bool enum_body = false;
    bool expect_enum_name = false;
    size_t enum_parentheses = 0;
    size_t enum_brackets = 0;
    size_t enum_braces = 0;

    for (size_t i = start; i < end; i++) {
        struct Token token = tokens.elements[i];

        if (token.kind == TOKEN_COMMENT || token.kind == TOKEN_PP_DIRECTIVE) {
            continue;
        }

        /*
         * A top-level tag in this declaration:
         *
         *     struct Node { ... };
         *     typedef struct Node Node;
         */
        if (braces == 0 &&
            (type_token_is(tokens, i, "struct") ||
             type_token_is(tokens, i, "union") ||
             type_token_is(tokens, i, "enum"))) {
            bool is_enum = type_token_is(tokens, i, "enum");
            size_t next = type_next_nontrivia(tokens, i + 1);

            if (next < end && tokens.elements[next].kind == TOKEN_IDENTIFIER) {
                struct Token name = tokens.elements[next];

                if (name.text_len == reference.text_len && asm_strncmp(name.text, reference.text, name.text_len)) {
                    return true;
                }

                next = type_next_nontrivia(tokens, next + 1);
            }

            if (is_enum && next < end && tokens.elements[next].kind == TOKEN_LBRACE) {
                enum_body = true;
                expect_enum_name = true;
                enum_parentheses = 0;
                enum_brackets = 0;
                enum_braces = 0;

                /* Continue at the first token inside the enum. */
                i = next;
                braces++;
                continue;
            }
        }

        if (enum_body) {
            if (expect_enum_name && token.kind == TOKEN_IDENTIFIER) {
                if (token.text_len == reference.text_len && asm_strncmp(token.text, reference.text, token.text_len)) {
                    return true;
                }

                expect_enum_name = false;
            }

            switch (token.kind) {
                case TOKEN_LPAREN:
                    enum_parentheses++;
                    break;

                case TOKEN_RPAREN:
                    if (enum_parentheses > 0) {
                        enum_parentheses--;
                    }
                    break;

                case TOKEN_LBRACKET:
                    enum_brackets++;
                    break;

                case TOKEN_RBRACKET:
                    if (enum_brackets > 0) {
                        enum_brackets--;
                    }
                    break;

                case TOKEN_LBRACE:
                    enum_braces++;
                    break;

                case TOKEN_RBRACE:
                    if (enum_braces > 0) {
                        enum_braces--;
                    } else {
                        enum_body = false;
                    }
                    break;

                case TOKEN_COMMA:
                    if (enum_parentheses == 0 && enum_brackets == 0 && enum_braces == 0) {
                        expect_enum_name = true;
                    }
                    break;

                default:
                    break;
            }
        }

        if (token.kind == TOKEN_LBRACE) {
            braces++;
        } else if (token.kind == TOKEN_RBRACE && braces > 0) {
            braces--;
        }
    }

    /* Check the aliases introduced by a typedef. */
    size_t i = 0;

    if (!typedef_declarators_start_get(tokens, declaration, &i)) {
        return false;
    }

    while (i < end) {
        size_t name_index = 0;

        if (!type_declarator_name_get(tokens, &i, end, &name_index)) {
            return false;
        }

        struct Token name = tokens.elements[name_index];

        if (name.text_len == reference.text_len &&
            asm_strncmp(name.text, reference.text, name.text_len)) {
            return true;
        }

        i = type_next_nontrivia(tokens, i);

        if (i >= end ||
            tokens.elements[i].kind != TOKEN_COMMA) {
            break;
        }

        i++;
    }

    return false;
}

static void type_declarations_mark_for_identifier(struct Lexed_Files files, struct Type_Declaration_Array declarations, struct Token reference, unsigned char *selected)
{
    /* By AI */
    if (reference.kind != TOKEN_IDENTIFIER) {
        return;
    }

    for (size_t i = 0; i < declarations.length; i++) {
        if (selected[i]) {
            continue;
        }

        struct Type_Declaration declaration = declarations.elements[i];
        struct Tokens tokens = files.elements[declaration.file_index];

        if (type_declaration_provides_name(tokens, declaration, reference)) {
            selected[i] = 1;
        }
    }
}

static void type_declarations_mark_for_range(struct Lexed_Files files, struct Type_Declaration_Array declarations, struct Tokens tokens, size_t start, size_t end, unsigned char *selected)
{
    /* By AI */
    for (size_t i = start; i <= end; i++) {
        type_declarations_mark_for_identifier(files, declarations, tokens.elements[i], selected);
    }
}

bool relevant_type_declarations_get(struct Lexed_Files files, struct Type_Declaration_Array all_types, struct Func_Def_Array selected_functions, struct Type_Declaration_Array *relevant_types)
{
    /* By AI */
    if (relevant_types == NULL) {
        return FAIL;
    }

    if (all_types.length == 0) {
        return SUCCESS;
    }

    unsigned char *selected = AL_MALLOC(all_types.length);

    if (selected == NULL) {
        al_dprintERROR("%s", "Could not allocate type-selection state.");
        return FAIL;
    }

    memset(selected, 0, all_types.length);

    /*
     * Seed dependencies from the entire selected function:
     * return type, parameters, and body.
     */
    for (size_t i = 0; i < selected_functions.length; i++) {
        struct Function_Definition function = selected_functions.elements[i];
        struct Tokens tokens = files.elements[function.file_index];

        type_declarations_mark_for_range(files, all_types, tokens, function.token_start_index, function.token_end_index, selected);
    }

    /*
     * Expand dependencies until every selected declaration
     * has been scanned.
     */
    for (;;) {
        bool scanned_any = false;

        for (size_t i = 0; i < all_types.length; i++) {
            /**
             * state = 0 -> not selected.
             * state = 1 -> selected dependencies not scanned.
             * state = 2 -> selected dependencies scanned.
             */
            if (selected[i] != 1) {
                continue;
            }

            selected[i] = 2;
            scanned_any = true;

            struct Type_Declaration declaration = all_types.elements[i];
            struct Tokens tokens = files.elements[declaration.file_index];

            type_declarations_mark_for_range(files, all_types, tokens, declaration.token_start_index, declaration.token_end_index, selected);
        }

        if (!scanned_any) {
            break;
        }
    }

    for (size_t i = 0; i < all_types.length; i++) {
        if (selected[i] != 0) {
            ada_appand(struct Type_Declaration, *relevant_types, all_types.elements[i]);
        }
    }

    AL_FREE(selected);
    return SUCCESS;
}

int main(int argc, char const *argv[])
{
    FILE *output_target = stdout;
    const char *output_file_path = NULL;
    const char *entry_file_relative_path = NULL;
    const char *entry_function_name = NULL;

    for (int i = 1; i < argc; i++) {
        if (asm_strncmp((char *)argv[i], "-o", ASM_MAX_LEN) ||
            asm_strncmp((char *)argv[i], "--output", ASM_MAX_LEN)) {
            if (i + 1 >= argc) {
                al_dprintERROR("%s", "Missing file path after -o/--output.");
                return -1;
            }
            output_file_path = argv[++i];
        } else if (entry_file_relative_path == NULL) {
            entry_file_relative_path = argv[i];
        } else if (entry_function_name == NULL) {
            entry_function_name = argv[i];
        } else {
            al_dprintERROR("Unexpected argument: '%s'", argv[i]);
            return -1;
        }
    }
    if (entry_file_relative_path == NULL || entry_function_name == NULL) {
        al_dprintERROR("Usage: %s [-o output_file] entry_file.c function_name", argv[0]);
        return -1;
    }
    if (output_file_path != NULL && !asm_strncmp((char *)output_file_path, "-", ASM_MAX_LEN)) {
        output_target = fopen(output_file_path, "w");
        if (output_target == NULL) {
            al_dprintERROR(
                "Could not open output file '%s' for writing.",
                output_file_path
            );
            return -1;
        }
    }

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
    if (FAIL == function_definitions_get_from_lexed_files(lexed_files, &func_def_array)) {
        al_dprintERROR("%s", "Could not get function definitions from lexes files.");
        return -1;
    }

    struct Type_Declaration_Array type_declarations = {0};
    ada_init_array(struct Type_Declaration, type_declarations);
    if (FAIL == type_declarations_get_from_lexed_files( lexed_files, func_def_array, &type_declarations)) {
        al_dprintERROR("%s", "Could not collect type declarations.");
        return -1;
    }

    struct Func_Call_Array func_call_array = {0};
    ada_init_array(struct Function_Call, func_call_array);
    func_call_array_get_from_func_def_array(lexed_files, func_def_array, &func_call_array);

    struct Macro_Def_Array macro_def_array = {0};
    ada_init_array(struct Macro_Definition, macro_def_array);
    if (FAIL == macro_definitions_get_from_lexed_files(lexed_files, &macro_def_array)) {
        al_dprintERROR("%s", "Could not get macro definitions.");
        return -1;
    }
    func_call_array_add_macro_calls(lexed_files, func_def_array, macro_def_array, &func_call_array);

    int entry_func_index = func_def_index_get_by_name(func_def_array, entry_function_name);
    if (entry_func_index < 0) {
        al_dprintERROR("Could not find function definition of entry function name '%s'.", entry_function_name);
        return -1;
    }

    struct Func_Def_Array func_defs_to_print = {0};
    ada_init_array(struct Function_Definition, func_defs_to_print);
    func_defs_to_print_get_from_func_def_index(func_def_array, func_call_array, entry_func_index, &func_defs_to_print);

    struct Type_Declaration_Array relevant_types = {0};
    ada_init_array(struct Type_Declaration, relevant_types);
    if (FAIL == relevant_type_declarations_get(lexed_files, type_declarations, func_defs_to_print, &relevant_types)) {
        al_dprintERROR("%s", "Could not select relevant type declarations.");
        return -1;
    }
    
    struct Macro_Def_Array used_macro_defs = {0};
    ada_init_array(struct Macro_Definition, used_macro_defs);
    macro_defs_used_in_func_def_array_get(lexed_files, func_defs_to_print, macro_def_array, &used_macro_defs);
    macro_defs_dependencies_get(lexed_files, macro_def_array, &used_macro_defs, func_def_array, -1, &func_call_array);
    
    /* printing to output target */
    macro_def_array_content_print_to_output_target(output_target, used_macro_defs, lexed_files);
    type_declarations_print_to_output_target(output_target, type_declarations, lexed_files);
    // type_declarations_print_to_output_target(output_target, relevant_types, lexed_files);
    func_def_array_content_print_to_output_target(output_target, func_defs_to_print, lexed_files);


    if (output_target != stdout) {
        fclose(output_target);
    }
    for (size_t i = 0; i <lexed_files.length; i++) {
        al_tokens_free(lexed_files.elements[i]);
    }
    AL_FREE(lexed_files.elements);
    AL_FREE(func_def_array.elements);
    AL_FREE(func_call_array.elements);
    AL_FREE(macro_def_array.elements);
    AL_FREE(used_macro_defs.elements);
    AL_FREE(func_defs_to_print.elements);
    AL_FREE(type_declarations.elements);
    AL_FREE(relevant_types.elements);

    // if (AMD_FAIL == amd_debug_mem()) {
    //     amd_dprintERROR("%s", "Corrupted memory detected.");
    //     return -1;
    // }
    // amd_debug_mem_print(0);

    return 0;
}
