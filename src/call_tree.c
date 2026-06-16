#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#define DIR_SEPARATOR '\\'
#include <sys/stat.h>
#define stat _stat
#define IS_DIR(mode) (((mode) & _S_IFDIR) != 0)
#define IS_REG(mode) (((mode) & _S_IFREG) != 0)
#else
#include <unistd.h>
#define DIR_SEPARATOR '/'
#include <sys/stat.h>
#define IS_DIR(mode) S_ISDIR(mode)
#define IS_REG(mode) S_ISREG(mode)
#endif

#define ALMOG_LEXER_IMPLEMENTATION
#define ALMOG_STRING_MANIPULATION_IMPLEMENTATION
#include "./includes/Almog_Lexer.h"

#define SUCCESS 1
#define OK SUCCESS
#define FAIL 0

typedef char Word[ASM_MAX_LEN];

#define WORD_ARRAY_MAX_LEN 100
struct Word_Array {
    size_t length;
    Word *elements;
};

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

struct Word_Array word_array_alloc() 
{
    struct Word_Array word_array = {
        .length = 0,
        .elements = (Word *)AL_MALLOC(WORD_ARRAY_MAX_LEN * sizeof(Word))
    };
    if (word_array.elements == NULL) {
        al_dprintERROR("%s", "Failed to allocate a word array.");
        exit(1);
    }

    return word_array;
}

#define word_array_print(word_array) do {al_dprintINFO("%s = ", #word_array); word_array_print_imp(word_array, 7);} while (0)
void word_array_print_imp(struct Word_Array wa, size_t padding)
{
    for (size_t i = 0; i < wa.length; i++) {
        printf("%*.s%s\n", (int)padding, "", wa.elements[i]);
    }
}

#define function_definition_print(tokens, func_def) do {al_dprintINFO("%s = ", #func_def); function_definition_print_imp(tokens, func_def, 11);} while (0)
void function_definition_print_imp(struct Tokens tokens, struct Function_Definition func_def, size_t padding)
{
    printf("%*.sname              -> %s\n", (int)padding, "", func_def.name);
    printf("%*.sfile name         -> %s\n", (int)padding, "", func_def.file_name);
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

#define func_def_array_print(func_def_array) do {al_dprintINFO("%s = ", #func_def_array); func_def_array_print_imp(func_def_array, 11);} while (0)
void func_def_array_print_imp (struct Func_Def_Array func_def_array, size_t padding)
{
    for (size_t i = 0; i < func_def_array.length; i++) {
        struct Function_Definition func_def = func_def_array.elements[i];
        printf("%*.sFunc No.%zu:\n", (int)padding, "", i);
        printf("%*.s    name       -> %s\n", (int)padding, "", func_def.name);
        printf("%*.s    file name  -> %s\n", (int)padding, "", func_def.file_name);
        printf("%*.s    file index -> %d\n", (int)padding, "", func_def.file_index);
    }
}

bool path_exists(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return SUCCESS;
    } else {
        return FAIL;
    }
}

bool path_is_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }
    if (IS_DIR(st.st_mode)) {
        return SUCCESS;
    } else {
        return FAIL;
    }
}

bool path_is_regular_file(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return FAIL;
    }
    if (IS_REG(st.st_mode)) {
        return SUCCESS;
    } else {
        return FAIL;
    }
}

bool path_is_absolute(const char *path)
{
    if (path == NULL || path[0] == '\0') {
        return FAIL;
    }

    if (asm_length(path) <= 2) {
        return FAIL;
    }

    #ifdef _WIN32
        /** Examples of absolute paths on Windows:
         * C:\file.txt
         * C:/file.txt
         * \\server\share
         * /some/path   (root-relative, often treated as absolute)
         */

        /* UNC path: \\server\share */
        if ((path[0] == '\\' && path[1] == '\\') ||
            (path[0] == '/' && path[1] == '/')) {
            return SUCCESS;
        }

        /* Drive letter path: C:\ or C:/ */
        if (asm_isalpha((unsigned char)path[0]) &&
            path[1] == ':' &&
            (path[2] == '\\' || path[2] == '/')) {
            return SUCCESS;
        }

        /* Root-relative path */
        if (path[0] == '\\' || path[0] == '/') {
            return SUCCESS;
        }

        return FAIL;
    #else
        /* On Unix/Linux, absolute paths start with / */
        if (path[0] == '/') {
            return SUCCESS;
        } else {
            return FAIL;
        }
    #endif
}

bool path_fix(char *path)
{
    /* Written by AI */
    if (path == NULL || path[0] == '\0') {
        return FAIL;
    }

    size_t len = asm_length(path);
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '\\' || path[i] == '/') {
            path[i] = DIR_SEPARATOR;
        }
    }

    Word original;
    Word result;
    Word segment;
    Word parts[WORD_ARRAY_MAX_LEN];

    asm_strncpy(original, path, ASM_MAX_LEN);
    result[0] = '\0';

    size_t part_count = 0;
    size_t i = 0;

    bool is_absolute = false;

    #ifdef _WIN32
        bool has_drive = false;
        char drive[4] = {0};

        if (len >= 3 &&
            asm_isalpha((unsigned char)original[0]) &&
            original[1] == ':' &&
            original[2] == DIR_SEPARATOR) {
            has_drive = true;
            is_absolute = true;
            drive[0] = original[0];
            drive[1] = ':';
            drive[2] = DIR_SEPARATOR;
            drive[3] = '\0';
            i = 3;
        } else if (len >= 2 &&
                original[0] == DIR_SEPARATOR &&
                original[1] == DIR_SEPARATOR) {
            /* UNC path - this implementation preserves leading // but does not
            fully validate/share-split UNC semantics */
            is_absolute = true;
            i = 2;
        } else if (original[0] == DIR_SEPARATOR) {
            is_absolute = true;
            i = 1;
        }
    #else
        if (original[0] == DIR_SEPARATOR) {
            is_absolute = true;
            i = 1;
        }
    #endif

    while (i <= len) {
        size_t j = 0;

        while (i < len && original[i] == DIR_SEPARATOR) {
            i++;
        }

        while (i < len && original[i] != DIR_SEPARATOR) {
            if (j + 1 >= ASM_MAX_LEN) {
                al_dprintERROR("%s", "Path segment too long.");
                return FAIL;
            }
            segment[j++] = original[i++];
        }
        segment[j] = '\0';

        if (j == 0) {
            break;
        }

        if (asm_strncmp(segment, ".", ASM_MAX_LEN)) {
            continue;
        }

        if (asm_strncmp(segment, "..", ASM_MAX_LEN)) {
            if (part_count > 0 &&
                !asm_strncmp(parts[part_count - 1], "..", ASM_MAX_LEN)) {
                part_count--;
            } else if (!is_absolute) {
                if (part_count >= WORD_ARRAY_MAX_LEN) {
                    al_dprintERROR("%s", "Too many path segments.");
                    return FAIL;
                }
                asm_strncpy(parts[part_count++], segment, ASM_MAX_LEN);
            }
            continue;
        }

        if (part_count >= WORD_ARRAY_MAX_LEN) {
            al_dprintERROR("%s", "Too many path segments.");
            return FAIL;
        }
        asm_strncpy(parts[part_count++], segment, ASM_MAX_LEN);
    }

    #ifdef _WIN32
        if (has_drive) {
            asm_strncpy(result, drive, ASM_MAX_LEN);
        } else if (is_absolute) {
            result[0] = DIR_SEPARATOR;
            result[1] = '\0';
        }
    #else
        if (is_absolute) {
            result[0] = DIR_SEPARATOR;
            result[1] = '\0';
        }
    #endif

    for (size_t k = 0; k < part_count; k++) {
        if (asm_length(result) > 0 &&
            result[asm_length(result) - 1] != DIR_SEPARATOR) {
            char sep[2] = {DIR_SEPARATOR, '\0'};
            asm_strncat(result, sep, 1);
        }
        asm_strncat(result, parts[k], ASM_MAX_LEN);
    }

    if (result[0] == '\0') {
        if (is_absolute) {
            result[0] = DIR_SEPARATOR;
            result[1] = '\0';
        } else {
            result[0] = '.';
            result[1] = '\0';
        }
    }

    asm_strncpy(path, result, ASM_MAX_LEN);
    return SUCCESS;
}

bool path_is_valid_file(const char *path)
{
    if (!path_exists(path)) {
        al_dprintERROR("Path does not exist: '%s'", path);
        return FAIL;
    }
    if (path_is_directory(path)) {
        al_dprintERROR("Expected a file, but got a directory: '%s'", path);
        return FAIL;
    }
    if (!path_is_regular_file(path)) {
        al_dprintERROR("Expected a regular file: '%s'", path);
        return FAIL;
    }

    return SUCCESS;
}

bool join_two_paths(char *des, char *p1, char *p2) 
{
    size_t p1_len = asm_length(p1);
    size_t p2_len = asm_length(p2);
    if (p1_len == 0) {
        asm_strncpy(des, p2, ASM_MAX_LEN);
    } else if (p2_len == 0) {
        asm_strncpy(des, p1, ASM_MAX_LEN);
    } else if (SUCCESS == path_is_absolute(p2)) {
        al_dprintWARNING("p2 is absolute. p2 = '%s'. Copied p2 to des.", p2);
        asm_strncpy(des, p2, ASM_MAX_LEN);
    } else if (p1[p1_len-1] == DIR_SEPARATOR) {
        snprintf(des, ASM_MAX_LEN, "%s%s", p1, p2);
    } else {
        snprintf(des, ASM_MAX_LEN, "%s%c%s", p1, DIR_SEPARATOR, p2);
    }

    if (FAIL == path_fix(des)) {
        al_dprintERROR("Could not fix path '%s'.", des);
        return FAIL;
    }

    return SUCCESS;
}

bool directory_get_from_path(char *dir_des, char *path) 
{
    size_t path_len = asm_length(path);
    if (path_len == 0) {
        dir_des[0] = '\0';
        return SUCCESS;
    }

    size_t last_separator_index = path_len;

    for (size_t i = path_len; i > 0; i--) {
        if (path[i - 1] == DIR_SEPARATOR) {
            last_separator_index = i - 1;
            break;
        }
    }
    if (last_separator_index == path_len) {
        al_dprintERROR("Failed to find dir separator in the path '%s'", path);
        return FAIL;
    }
    for (size_t i = 0; i <= last_separator_index; i++) {
        dir_des[i] = path[i];
    }
    dir_des[last_separator_index+1] = '\0';

    return SUCCESS;
}

bool include_paths_get_from_tokens(struct Word_Array *word_array, struct Tokens tokens)
{
    Word temp_word;
    Word current_directive;
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
                    if (FAIL == path_fix(temp_word)) {
                        al_dprintERROR("Could not fix path '%s'.", temp_word);
                        return FAIL;
                    }
                    if (word_array->length < WORD_ARRAY_MAX_LEN) {
                        asm_strncpy(word_array->elements[word_array->length++], temp_word, ASM_MAX_LEN);
                    } else {
                        al_dprintERROR("Could not add word to word array. Capacity is %d and the current length is %zu", WORD_ARRAY_MAX_LEN, word_array->length);
                        return FAIL;
                    }
                }
            }
        }
    }
    return SUCCESS;
}

bool paths_add_prefix(struct Word_Array path_array, char * prefix)
{
    for (size_t i = 0; i < path_array.length; i++) {
        Word current_word;
        Word joined;

        asm_strncpy(current_word, path_array.elements[i], ASM_MAX_LEN);
        if (FAIL == join_two_paths(joined, prefix, current_word)) {
            al_dprintERROR("Could not join path2 '%s' to path1 '%s'.", current_word, prefix);
            return FAIL;
        }
        asm_strncpy(path_array.elements[i], joined, ASM_MAX_LEN);
    }

    return SUCCESS;
}

bool includes_path_get_from_lexed_file(struct Word_Array *include_paths, struct Tokens lexed_file)
{
    if (FAIL == include_paths_get_from_tokens(include_paths, lexed_file)) {
        al_dprintERROR("Failed to get the include paths from the tokens of '%s'.", lexed_file.file_path);
        return FAIL;
    }
    Word entry_file_absolute_dir;
    if (FAIL == directory_get_from_path(entry_file_absolute_dir, lexed_file.file_path))
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
    if (FAIL == path_exists(path)) {
        al_dprintERROR("Path does not exist: '%s'", path);
        return FAIL;
    }
    if (FAIL == path_is_absolute(path)) {
        al_dprintERROR("Inputted path is not absolute '%s'.", path);
        return FAIL;
    }
    if (SUCCESS == path_is_directory(path)) {
        al_dprintERROR("Expected a file, but got a directory: '%s'.", path);
        return FAIL;
    }
    struct Tokens tokens = al_lex_entire_file(path);
    ada_appand(struct Tokens, *lexed_files, tokens);
    struct Word_Array nested_include_paths = {0};
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
    if (FAIL == path_is_valid_file(entry_file_relative_path)) {
        al_dprintERROR("%s", "Entry file is not a valid file.");
        return -1;
    }

    Word current_working_directory;
    if (getcwd(current_working_directory, sizeof(current_working_directory)) == NULL) {
        al_dprintERROR("%s", "Could not get current working directory.");
        return -1;
    }

    Word entry_file_absolute_path;
    if (SUCCESS == path_is_absolute(entry_file_relative_path)) {
        asm_strncpy(entry_file_absolute_path, entry_file_relative_path, ASM_MAX_LEN);
    } else {
        if (FAIL == join_two_paths(entry_file_absolute_path, current_working_directory, (char *)entry_file_relative_path)) {
            al_dprintERROR("Could not join path2 '%s' to path1 '%s'.", (char *)entry_file_relative_path, current_working_directory);
            return -1;
        }
        if (FAIL == path_is_absolute(entry_file_absolute_path)) {
            al_dprintERROR("Could not create an absolute path to the entry file. Created: '%s'", entry_file_absolute_path);
            return -1;
        }
    }
    if (FAIL == path_fix(entry_file_absolute_path)) {
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
    func_def_array_print(func_def_array);


    for (size_t i = 0; i <lexed_files.length; i++) {
        al_tokens_free(lexed_files.elements[i]);
    }
    AL_FREE(lexed_files.elements);
    AL_FREE(func_def_array.elements);
    return 0;
}
