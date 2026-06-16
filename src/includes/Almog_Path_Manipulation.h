#ifndef ALMOG_PATH_MANIPULATION_H_
#define ALMOG_PATH_MANIPULATION_H_

#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#define APM_DIR_SEPARATOR '\\'
#include <sys/stat.h>
#define stat _stat
#define IS_DIR(mode) (((mode) & _S_IFDIR) != 0)
#define IS_REG(mode) (((mode) & _S_IFREG) != 0)
#else
#include <unistd.h>
#define APM_DIR_SEPARATOR '/'
#include <sys/stat.h>
#define IS_DIR(mode) S_ISDIR(mode)
#define IS_REG(mode) S_ISREG(mode)
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef APM_MAX_LEN
#define APM_MAX_LEN (int)1e3
#endif
typedef char Apm_Word[APM_MAX_LEN];

#define APM_WORD_ARRAY_MAX_LEN 100
struct Apm_Word_Array {
    size_t length;
    Apm_Word *elements;
};

/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define apm_dprintSTRING(expr) printf("[Info] %s:%d:\n%*s" #expr " = %s\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define apm_dprintCHAR(expr) printf("[Info] %s:%d:\n%*s" #expr " = %c\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define apm_dprintINT(expr) printf("[Info] %s:%d:\n%*s" #expr " = %d\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define apm_dprintFLOAT(expr) printf("[Info] %s:%d:\n%*s" #expr " = %#f\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define apm_dprintDOUBLE(expr) printf("[Info] %s:%d:\n%*s" #expr " = %#g\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define apm_dprintSIZE_T(expr) printf("[Info] %s:%d:\n%*s" #expr " = %zu\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO variants print to
 * stdout and include file, line, and function information.
 */
#define apm_dprintINFO(fmt, ...) \
    fprintf(stdout, "[Info] %s:%d:\n%*sIn function '%s':\n%*s" fmt "\n", __FILE__, __LINE__, 7, "", __func__, 7, "", __VA_ARGS__)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define apm_dprintWARNING(fmt, ...) \
    fprintf(stderr, "[Warning] %s:%d:\n%*sIn function '%s':\n%*s" fmt "\n", __FILE__, __LINE__, 10, "", __func__, 10, "", __VA_ARGS__)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define apm_dprintERROR(fmt, ...) \
    fprintf(stderr, "[Error] %s:%d:\n%*sIn function '%s':\n%*s" fmt "\n", __FILE__, __LINE__, 8, "", __func__, 8, "", __VA_ARGS__)

#define apm_min(a, b) ((a) < (b) ? (a) : (b))

#ifndef APM_SUCCESS 
    #define APM_SUCCESS 1
#endif
#ifndef APM_FAIL
    #define APM_FAIL 0
#endif

#ifndef APM_DEF
    #ifdef APM_DEF_STATIC
        #define APM_DEF static
    #else
        #define APM_DEF extern
    #endif
#endif

APM_DEF bool    apm_directory_get_from_path(char *dir_des, char *path);
APM_DEF bool    apm_isalpha(char c);
APM_DEF bool    apm_islower(char c);
APM_DEF bool    apm_isupper(char c);
APM_DEF bool    apm_join_two_paths(char *des, char *p1, char *p2);
APM_DEF size_t  apm_length(const char * const str);
APM_DEF bool    apm_paths_add_prefix(struct Apm_Word_Array path_array, char * prefix);
APM_DEF bool    apm_path_exists(const char *path);
APM_DEF bool    apm_path_fix(char *path);
APM_DEF bool    apm_path_is_absolute(const char *path);
APM_DEF bool    apm_path_is_directory(const char *path);
APM_DEF bool    apm_path_is_regular_file(const char *path);
APM_DEF bool    apm_path_is_valid_file(const char *path);
APM_DEF int     apm_strncat(char * const s1, const char * const s2, const size_t N);
APM_DEF int     apm_strncmp(const char *s1, const char *s2, const size_t N);
APM_DEF int     apm_strncpy(char * const s1, const char * const s2, const size_t N);

#endif /*ALMOG_PATH_MANIPULATION_H_*/

#ifdef ALMOG_PATH_MANIPULATION_IMPLEMENTATION
#undef ALMOG_PATH_MANIPULATION_IMPLEMENTATION

APM_DEF bool apm_directory_get_from_path(char *dir_des, char *path) 
{
    size_t path_len = apm_length(path);
    if (path_len == 0) {
        dir_des[0] = '\0';
        return APM_SUCCESS;
    }

    size_t last_separator_index = path_len;

    for (size_t i = path_len; i > 0; i--) {
        if (path[i - 1] == APM_DIR_SEPARATOR) {
            last_separator_index = i - 1;
            break;
        }
    }
    if (last_separator_index == path_len) {
        apm_dprintERROR("Failed to find dir separator in the path '%s'", path);
        return APM_FAIL;
    }
    for (size_t i = 0; i <= last_separator_index; i++) {
        dir_des[i] = path[i];
    }
    dir_des[last_separator_index+1] = '\0';

    return APM_SUCCESS;
}

APM_DEF bool apm_isalpha(char c)
{
    return apm_isupper(c) || apm_islower(c);
}

APM_DEF bool apm_islower(char c)
{
    if (c >= 'a' && c <= 'z') {
        return true;
    } else {
        return false;
    }
}

APM_DEF bool apm_isupper(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return true;
    } else {
        return false;
    }
}

APM_DEF bool apm_join_two_paths(char *des, char *p1, char *p2) 
{
    size_t p1_len = apm_length(p1);
    size_t p2_len = apm_length(p2);
    if (p1_len == 0) {
        apm_strncpy(des, p2, APM_MAX_LEN);
    } else if (p2_len == 0) {
        apm_strncpy(des, p1, APM_MAX_LEN);
    } else if (APM_SUCCESS == apm_path_is_absolute(p2)) {
        apm_dprintWARNING("p2 is absolute. p2 = '%s'. Copied p2 to des.", p2);
        apm_strncpy(des, p2, APM_MAX_LEN);
    } else if (p1[p1_len-1] == APM_DIR_SEPARATOR) {
        snprintf(des, APM_MAX_LEN, "%s%s", p1, p2);
    } else {
        snprintf(des, APM_MAX_LEN, "%s%c%s", p1, APM_DIR_SEPARATOR, p2);
    }

    if (APM_FAIL == apm_path_fix(des)) {
        apm_dprintERROR("Could not fix path '%s'.", des);
        return APM_FAIL;
    }

    return APM_SUCCESS;
}

APM_DEF size_t apm_length(const char * const str)
{
    char c;
    size_t i = 0;

    while ((c = str[i++]) != '\0') {
        if (i > APM_MAX_LEN) {
            #ifndef NO_ERRORS
            apm_dprintERROR("%s", "index exceeds APM_MAX_LEN. Probably no NULL termination.");
            #endif
            return SIZE_MAX;
        }
    }
    return --i;
}

APM_DEF bool apm_paths_add_prefix(struct Apm_Word_Array path_array, char * prefix)
{
    for (size_t i = 0; i < path_array.length; i++) {
        Apm_Word current_word;
        Apm_Word joined;

        apm_strncpy(current_word, path_array.elements[i], APM_MAX_LEN);
        if (APM_FAIL == apm_join_two_paths(joined, prefix, current_word)) {
            apm_dprintERROR("Could not join path2 '%s' to path1 '%s'.", current_word, prefix);
            return APM_FAIL;
        }
        apm_strncpy(path_array.elements[i], joined, APM_MAX_LEN);
    }

    return APM_SUCCESS;
}

APM_DEF bool apm_path_exists(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return APM_SUCCESS;
    } else {
        return APM_FAIL;
    }
}

APM_DEF bool apm_path_fix(char *path)
{
    /* Written by AI */
    if (path == NULL || path[0] == '\0') {
        return APM_FAIL;
    }

    size_t len = apm_length(path);
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '\\' || path[i] == '/') {
            path[i] = APM_DIR_SEPARATOR;
        }
    }

    Apm_Word original;
    Apm_Word result;
    Apm_Word segment;
    Apm_Word parts[APM_WORD_ARRAY_MAX_LEN];

    apm_strncpy(original, path, APM_MAX_LEN);
    result[0] = '\0';

    size_t part_count = 0;
    size_t i = 0;

    bool is_absolute = false;

    #ifdef _WIN32
        bool has_drive = false;
        char drive[4] = {0};

        if (len >= 3 &&
            apm_isalpha((unsigned char)original[0]) &&
            original[1] == ':' &&
            original[2] == APM_DIR_SEPARATOR) {
            has_drive = true;
            is_absolute = true;
            drive[0] = original[0];
            drive[1] = ':';
            drive[2] = APM_DIR_SEPARATOR;
            drive[3] = '\0';
            i = 3;
        } else if (len >= 2 &&
                original[0] == APM_DIR_SEPARATOR &&
                original[1] == APM_DIR_SEPARATOR) {
            /* UNC path - this implementation preserves leading // but does not
            fully validate/share-split UNC semantics */
            is_absolute = true;
            i = 2;
        } else if (original[0] == APM_DIR_SEPARATOR) {
            is_absolute = true;
            i = 1;
        }
    #else
        if (original[0] == APM_DIR_SEPARATOR) {
            is_absolute = true;
            i = 1;
        }
    #endif

    while (i <= len) {
        size_t j = 0;

        while (i < len && original[i] == APM_DIR_SEPARATOR) {
            i++;
        }

        while (i < len && original[i] != APM_DIR_SEPARATOR) {
            if (j + 1 >= APM_MAX_LEN) {
                apm_dprintERROR("%s", "Path segment too long.");
                return APM_FAIL;
            }
            segment[j++] = original[i++];
        }
        segment[j] = '\0';

        if (j == 0) {
            break;
        }

        if (apm_strncmp(segment, ".", APM_MAX_LEN)) {
            continue;
        }

        if (apm_strncmp(segment, "..", APM_MAX_LEN)) {
            if (part_count > 0 &&
                !apm_strncmp(parts[part_count - 1], "..", APM_MAX_LEN)) {
                part_count--;
            } else if (!is_absolute) {
                if (part_count >= APM_WORD_ARRAY_MAX_LEN) {
                    apm_dprintERROR("%s", "Too many path segments.");
                    return APM_FAIL;
                }
                apm_strncpy(parts[part_count++], segment, APM_MAX_LEN);
            }
            continue;
        }

        if (part_count >= APM_WORD_ARRAY_MAX_LEN) {
            apm_dprintERROR("%s", "Too many path segments.");
            return APM_FAIL;
        }
        apm_strncpy(parts[part_count++], segment, APM_MAX_LEN);
    }

    #ifdef _WIN32
        if (has_drive) {
            apm_strncpy(result, drive, APM_MAX_LEN);
        } else if (is_absolute) {
            result[0] = APM_DIR_SEPARATOR;
            result[1] = '\0';
        }
    #else
        if (is_absolute) {
            result[0] = APM_DIR_SEPARATOR;
            result[1] = '\0';
        }
    #endif

    for (size_t k = 0; k < part_count; k++) {
        if (apm_length(result) > 0 &&
            result[apm_length(result) - 1] != APM_DIR_SEPARATOR) {
            char sep[2] = {APM_DIR_SEPARATOR, '\0'};
            apm_strncat(result, sep, 1);
        }
        apm_strncat(result, parts[k], APM_MAX_LEN);
    }

    if (result[0] == '\0') {
        if (is_absolute) {
            result[0] = APM_DIR_SEPARATOR;
            result[1] = '\0';
        } else {
            result[0] = '.';
            result[1] = '\0';
        }
    }

    apm_strncpy(path, result, APM_MAX_LEN);
    return APM_SUCCESS;
}

APM_DEF bool apm_path_is_absolute(const char *path)
{
    if (path == NULL || path[0] == '\0') {
        return APM_FAIL;
    }

    if (apm_length(path) <= 2) {
        return APM_FAIL;
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
            return APM_SUCCESS;
        }

        /* Drive letter path: C:\ or C:/ */
        if (apm_isalpha((unsigned char)path[0]) &&
            path[1] == ':' &&
            (path[2] == '\\' || path[2] == '/')) {
            return APM_SUCCESS;
        }

        /* Root-relative path */
        if (path[0] == '\\' || path[0] == '/') {
            return APM_SUCCESS;
        }

        return APM_FAIL;
    #else
        /* On Unix/Linux, absolute paths start with / */
        if (path[0] == '/') {
            return APM_SUCCESS;
        } else {
            return APM_FAIL;
        }
    #endif
}

APM_DEF bool apm_path_is_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return false;
    }
    if (IS_DIR(st.st_mode)) {
        return APM_SUCCESS;
    } else {
        return APM_FAIL;
    }
}

APM_DEF bool apm_path_is_regular_file(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return APM_FAIL;
    }
    if (IS_REG(st.st_mode)) {
        return APM_SUCCESS;
    } else {
        return APM_FAIL;
    }
}

APM_DEF bool apm_path_is_valid_file(const char *path)
{
    if (!apm_path_exists(path)) {
        apm_dprintERROR("Path does not exist: '%s'", path);
        return APM_FAIL;
    }
    if (apm_path_is_directory(path)) {
        apm_dprintERROR("Expected a file, but got a directory: '%s'", path);
        return APM_FAIL;
    }
    if (!apm_path_is_regular_file(path)) {
        apm_dprintERROR("Expected a regular file: '%s'", path);
        return APM_FAIL;
    }

    return APM_SUCCESS;
}

APM_DEF int apm_strncat(char * const s1, const char * const s2, const size_t N)
{
    size_t len_s1 = apm_length(s1);

    int limit = (int)N;
    if (limit == 0) {
        limit = APM_MAX_LEN;
    }

    int i = 0;
    while (i < limit && s2[i] != '\0') {
        if (len_s1 + (size_t)i >= APM_MAX_LEN-1) {
            #ifndef NO_ERRORS
            apm_dprintERROR("s2 or the first N=%zu digit of s2 does not fit into s1.", N);
            #endif
            return i;
        }

        s1[len_s1+(size_t)i] = s2[i];
        i++;
    }
    s1[len_s1+(size_t)i] = '\0';

    return i;
}

APM_DEF int apm_strncmp(const char *s1, const char *s2, const size_t N)
{
    size_t i = 0;
    while (i < N) {
        if (s1[i] == '\0' && s2[i] == '\0') {
            break;
        }
        if (s1[i] != s2[i] || (s1[i] == '\0') || (s2[i] == '\0')) {
            return 0;
        }
        i++;
    }
    return 1;
}

APM_DEF int apm_strncpy(char * const s1, const char * const s2, const size_t N)
{
    if (N == 0) return 0;

    size_t n = apm_min(N, (size_t)APM_MAX_LEN - 1);

    size_t i;
    for (i = 0; i < n && s2[i] != '\0'; i++) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';

    return (int)i;
}


#endif /*ALMOG_PATH_MANIPULATION_IMPLEMENTATION*/