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
struct Word_array {
    size_t length;
    Word *elements;
};
struct Lexed_Files {
    size_t length;
    size_t capacity;
    struct Tokens *elements;
};

struct Word_array word_array_alloc() 
{
    struct Word_array word_array = {
        .length = 0,
        .elements = (Word *)AL_MALLOC(WORD_ARRAY_MAX_LEN * sizeof(Word))
    };
    if (word_array.elements == NULL) {
        al_dprintERROR("%s", "Failed to allocate a word array.");
        exit(1);
    }

    return word_array;
}

#define word_array_print(word_array) al_dprintINFO("%s = ", #word_array); word_array_print_imp(word_array, 7);
void word_array_print_imp(struct Word_array wa, size_t padding)
{
    for (size_t i = 0; i < wa.length; i++) {
        printf("%*.s%s\n", (int)padding, "", wa.elements[i]);
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

void path_fix(char *path)
{
    size_t len = asm_length(path);
    if (len == 0) return;

    for (size_t i = 0; i < len; i++) {
        if (path[i] == '\\' || path[i] == '/') {
            path[i] = DIR_SEPARATOR;
        }
    }
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

void join_two_paths(char *des, char *p1, char *p2) 
{
    size_t p1_len = asm_length(p1);
    size_t p2_len = asm_length(p2);
    if (p1_len == 0) {
        asm_strncpy(des, p2, ASM_MAX_LEN);
    } else if (p2_len == 0) {
        asm_strncpy(des, p1, ASM_MAX_LEN);
    } else if (SUCCESS == path_is_absolute(p2)) {
        al_dprintWARNING("p2 is absolute. p2 = '%s'", p2);
    } else if (p1[p1_len-1] == DIR_SEPARATOR) {
        snprintf(des, ASM_MAX_LEN, "%s%s", p1, p2);
    } else {
        snprintf(des, ASM_MAX_LEN, "%s%c%s", p1, DIR_SEPARATOR, p2);
    }

}

bool directory_get_from_path(char *dir_des, char *path) 
{
    size_t path_len = asm_length(path);
    if (path_len == 0) return SUCCESS;

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

bool include_paths_get_from_tokens(struct Word_array *word_array, struct Tokens tokens)
{
    *word_array = word_array_alloc();

    Word temp_word;
    Word current_directive;
    for (size_t i = 0; i < tokens.length; i++) {
        struct Token token = tokens.elements[i];
        if (token.kind == TOKEN_PP_DIRECTIVE) {
            asm_strncpy(current_directive, token.text, token.text_len);
            asm_get_token_and_cut(temp_word, current_directive, ' ', false);
            if (asm_strncmp(temp_word, "#include", ASM_MAX_LEN)) {
                if (current_directive[0] == '"') {
                    asm_get_token_and_cut(temp_word, current_directive, '"', false);
                    asm_get_token_and_cut(temp_word, current_directive, '"', false);
                    path_fix(temp_word);
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

void word_array_add_prefix(struct Word_array word_array, char * prefix)
{
    for (size_t i = 0; i < word_array.length; i++) {
        Word current_word;
        asm_strncpy(current_word, word_array.elements[i], ASM_MAX_LEN);
        snprintf(word_array.elements[i], ASM_MAX_LEN, "%s%s", prefix, current_word);
    }
}

bool includes_path_get_from_lexed_file(struct Word_array *include_paths, struct Tokens lexed_file)
{
    include_paths_get_from_tokens(include_paths, lexed_file);
    Word entry_file_absolute_dir;
    if (FAIL == directory_get_from_path(entry_file_absolute_dir, lexed_file.file_path))
    {
        al_dprintERROR("Failed to get the absolute path of '%s'", lexed_file.file_path);
        return FAIL;
    }
    word_array_add_prefix(*include_paths, entry_file_absolute_dir);

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
    if (FAIL == path_is_directory(path)) {
        al_dprintERROR("Expected a file, but got a directory: '%s'.", path);
        return FAIL;
    }
    struct Tokens tokens = al_lex_entire_file(path);
    ada_appand(struct Tokens, *lexed_files, tokens);
    struct Word_array nested_include_paths = {0};
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
            al_dprintWARNING("Detected repeated inclusion. File: '%s' includes '%s' that was already incountred.", path, nested_include_paths.elements[i]);
        }
    }
    // al_dprintINFO("In file %s:", path);
    // word_array_print(nested_include_paths);

    AL_FREE(nested_include_paths.elements);
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
    join_two_paths(entry_file_absolute_path, current_working_directory, (char *)entry_file_relative_path);
    if (FAIL == path_is_absolute(entry_file_absolute_path)) {
        al_dprintERROR("Could not create an absolute path to the entry file. Created: '%s'", entry_file_absolute_path);
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
    


    AL_FREE(lexed_files.elements);
    return 0;
}
