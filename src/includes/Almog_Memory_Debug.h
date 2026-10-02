/**
 * This simple memory debugger is heavily inspired by Eskil Steenberg memory debugger.
 * You can find his memory debugger here: https://github.com/valiet/quel_solaar/blob/main/Forge/f_mem_debug.c.
 * He talked about his code in this video: https://youtu.be/sfrnU3-EpPI?t=1552.
 */

#ifndef ALMOG_MEMORY_DEBUG_H_
#define ALMOG_MEMORY_DEBUG_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#ifndef AMD_MALLOC
#include <stdlib.h>
#define AMD_MALLOC malloc
#endif /*AMD_MALLOC*/
#ifndef AMD_EXIT
#include <stdlib.h>
#define AMD_EXIT exit
#endif /*AMD_EXIT*/
#ifndef AMD_REALLOC
#include <stdlib.h>
#define AMD_REALLOC realloc
#endif /*AMD_REALLOC*/
#ifndef AMD_FREE
#include <stdlib.h>
#define AMD_FREE free
#endif /*AMD_FREE*/

/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define amd_dprintSTRING(expr) printf("[Info] %s:%d:\n%*s" #expr " = %s\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define amd_dprintCHAR(expr) printf("[Info] %s:%d:\n%*s" #expr " = %c\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define amd_dprintINT(expr) printf("[Info] %s:%d:\n%*s" #expr " = %d\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define amd_dprintFLOAT(expr) printf("[Info] %s:%d:\n%*s" #expr " = %#f\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define amd_dprintDOUBLE(expr) printf("[Info] %s:%d:\n%*s" #expr " = %#g\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO/WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define amd_dprintSIZE_T(expr) printf("[Info] %s:%d:\n%*s" #expr " = %zu\n", __FILE__, __LINE__, 7, "", expr)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The INFO variants print to
 * stdout and include file, line, and function information.
 */
#define amd_dprintINFO(fmt, ...) \
    fprintf(stdout, "[Info] %s:%d:\n%*sIn function '%s':\n%*s" fmt "\n", __FILE__, __LINE__, 7, "", __func__, 7, "", __VA_ARGS__)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define amd_dprintWARNING(fmt, ...) \
    fprintf(stderr, "[Warning] %s:%d:\n%*sIn function '%s':\n%*s" fmt "\n", __FILE__, __LINE__, 10, "", __func__, 10, "", __VA_ARGS__)
/**
 * @name Debug-print helpers
 * @brief Convenience macros for diagnostic output.
 *
 * The typed variants print to stdout. The WARNING/ERROR variants print to
 * stderr and include file, line, and function information.
 */
#define amd_dprintERROR(fmt, ...) \
    fprintf(stderr, "[Error] %s:%d:\n%*sIn function '%s':\n%*s" fmt "\n", __FILE__, __LINE__, 8, "", __func__, 8, "", __VA_ARGS__)


/* ADA */
#ifndef AMD_ADA_INIT_CAPACITY
#define AMD_ADA_INIT_CAPACITY 10
#endif /*AMD_ADA_INIT_CAPACITY*/
#ifndef AMD_ADA_MALLOC
#define AMD_ADA_MALLOC AMD_MALLOC
#endif /*AMD_ADA_MALLOC*/
#ifndef AMD_ADA_EXIT
#define AMD_ADA_EXIT AMD_EXIT
#endif /*AMD_ADA_EXIT*/
#ifndef AMD_ADA_REALLOC
#define AMD_ADA_REALLOC AMD_REALLOC
#endif /*AMD_ADA_REALLOC*/
#ifndef AMD_ADA_ASSERT
#include <assert.h>
#define AMD_ADA_ASSERT assert
#endif /*AMD_ADA_ASSERT*/
#define amd_ada_init_array(type, header) do {                                           \
        (header).capacity = AMD_ADA_INIT_CAPACITY;                                      \
        (header).length = 0;                                                        \
        (header).elements = (type *)AMD_ADA_MALLOC(sizeof(type) * (header).capacity);   \
        if ((header).elements == NULL) {                                            \
            AMD_ADA_EXIT(1);                                                            \
        }                                                                           \
    } while (0)
#define amd_ada_resize(type, header, new_capacity) do {                                                         \
        type *amd_ada_temp_pointer = (type *)AMD_ADA_REALLOC((void *)((header).elements), new_capacity*sizeof(type)); \
        if (amd_ada_temp_pointer == NULL) {                                                                     \
            AMD_ADA_EXIT(1);                                                                                        \
        }                                                                                                   \
        (header).elements = amd_ada_temp_pointer;                                                                 \
        AMD_ADA_ASSERT((header).elements != NULL);                                                                \
        (header).capacity = new_capacity;                                                                     \
    } while (0)
#define amd_ada_appand(type, header, value) do {                                            \
        if ((header).length >= (header).capacity) {                                         \
            amd_ada_resize(type, (header), (int)((header).capacity + (header).capacity/2 + 1));   \
        }                                                                               \
        (header).elements[(header).length] = value;                                         \
        (header).length++;                                                                \
    } while (0)
#define amd_ada_insert(type, header, value, index) do {                                                             \
    AMD_ADA_ASSERT((int)(index) >= 0);                                                                              \
    AMD_ADA_ASSERT((float)(index) - (int)(index) == 0);                                                             \
    AMD_ADA_ASSERT((header).length > 0 && "You can not insert to an empty array.");                                 \
    AMD_ADA_ASSERT(index <= (header).length);                                                                       \
    amd_ada_appand(type, (header), (header).elements[(header).length-1]);                                           \
    for (int amd_ada_for_loop_index = (header).length-2; amd_ada_for_loop_index > (int)(index); amd_ada_for_loop_index--) { \
        (header).elements[amd_ada_for_loop_index] = (header).elements [amd_ada_for_loop_index-1];                       \
    }                                                                                                           \
    (header).elements[(index)] = value;                                                                         \
} while (0)
#define amd_ada_insert_unordered(type, header, value, index) do {   \
    AMD_ADA_ASSERT((int)(index) >= 0);                              \
    AMD_ADA_ASSERT((float)(index) - (int)(index) == 0);             \
    AMD_ADA_ASSERT(index <= (header).length);                       \
    if ((size_t)(index) == (header).length) {                   \
        amd_ada_appand(type, (header), value);                      \
    } else {                                                    \
        amd_ada_appand(type, (header), (header).elements[(index)]); \
        (header).elements[(index)] = value;                     \
    }                                                           \
} while (0)
#define amd_ada_remove(type, header, index) do {                                                                    \
    AMD_ADA_ASSERT((int)(index) >= 0);                                                                              \
    AMD_ADA_ASSERT((header).length > 0 && "You can not remove from an empty array.");                               \
    AMD_ADA_ASSERT((float)(index) - (int)(index) == 0);                                                             \
    AMD_ADA_ASSERT(index < (header).length);                                                                        \
    for (size_t amd_ada_for_loop_index = (index); amd_ada_for_loop_index < (header).length-1; amd_ada_for_loop_index++) {   \
        (header).elements[amd_ada_for_loop_index] = (header).elements[amd_ada_for_loop_index+1];                        \
    }                                                                                                           \
    (header).length--;                                                                                          \
} while (0)
#define amd_ada_remove_unordered(type, header, index) do {                              \
    AMD_ADA_ASSERT((int)(index) >= 0);                                                  \
    AMD_ADA_ASSERT((header).length > 0 && "You can not remove from an empty array.");   \
    AMD_ADA_ASSERT(index < (header).length);                                            \
    AMD_ADA_ASSERT((float)(index) - (int)(index) == 0);                                 \
    (header).elements[index] = (header).elements[(header).length-1];                \
    (header).length--;                                                              \
} while (0)
/* ADA */

struct Amd_Mem_Alloc_Buf {
    size_t size;
    void *buf;
};

#define AMD_MAX_LEN (int)1e3
struct Amd_Mem_Alloc_Line {
    size_t capacity;
    size_t length;
    struct Amd_Mem_Alloc_Buf *elements;
    size_t line;
    char file[AMD_MAX_LEN];
    size_t size;
    size_t allocations_over_all;
    size_t bytes_allocated_over_all;
    size_t freed;
};

#define AMD_MEM_ALLOC_LINES_MAX_NUM 1024
#define AMD_MEM_OVER_ALLOC 32
#define AMD_MEM_MAGIC_NUMBER 132

#define amd_min(a, b) ((a) < (b) ? (a) : (b))

#ifndef AMD_SUCCESS 
    #define AMD_SUCCESS 1
#endif
#ifndef AMD_FAIL
    #define AMD_FAIL 0
#endif

#ifndef AMD_DEF
    #ifdef AMD_DEF_STATIC
        #define AMD_DEF static
    #else
        #define AMD_DEF extern
    #endif
#endif

AMD_DEF bool    amd_debug_mem(void);
AMD_DEF void    amd_debug_mem_add(void *p, size_t size, char *file, size_t line);
AMD_DEF size_t  amd_debug_mem_consumption(void);
AMD_DEF void    amd_debug_mem_free(void *buf);
AMD_DEF void *  amd_debug_mem_malloc(size_t size, char *file, size_t line);
AMD_DEF void    amd_debug_mem_print(size_t min_allocs);
AMD_DEF void *  amd_debug_mem_realloc(void *p, size_t size, char *file, size_t line);
AMD_DEF bool    amd_debug_mem_remove(void *buf);
AMD_DEF void    amd_debug_mem_reset(void);
AMD_DEF int     amd_strncmp(const char *s1, const char *s2, const size_t N);
AMD_DEF int     amd_strncpy(char * const s1, const char * const s2, const size_t N);

#endif /*ALMOG_MEMORY_DEBUG_H_*/

#ifdef ALMOG_MEMORY_DEBUG_IMPLEMENTATION
#undef ALMOG_MEMORY_DEBUG_IMPLEMENTATION

struct Amd_Mem_Alloc_Line mem_alloc_lines[AMD_MEM_ALLOC_LINES_MAX_NUM];
size_t mem_alloc_lines_count = 0;

AMD_DEF bool amd_debug_mem(void)
{
    bool output = AMD_SUCCESS;
    size_t i, j, k;
    for (i = 0; i < mem_alloc_lines_count; i++) {
        for (j = 0; j < mem_alloc_lines[i].length; j++) {
            uint8_t *buf = mem_alloc_lines[i].elements[j].buf;
            size_t size = mem_alloc_lines[i].elements[j].size;
            for (k = 0; k < AMD_MEM_OVER_ALLOC; k++) {
                if (buf[size + k] != AMD_MEM_MAGIC_NUMBER) {
                    break;
                }
            }
            if (k < AMD_MEM_OVER_ALLOC) {
                amd_dprintERROR("MEM ERROR: overshoot at %s:%zu.", mem_alloc_lines[i].file, mem_alloc_lines[i].line);
                output = AMD_FAIL;
            }
        }
    }

    return output;
}

AMD_DEF void amd_debug_mem_add(void *p, size_t size, char *file, size_t line)
{
    size_t i;
    for (i = 0; i < AMD_MEM_OVER_ALLOC; i++) {
        ((uint8_t *)p)[size + i] = AMD_MEM_MAGIC_NUMBER;
    }
    if (mem_alloc_lines_count >= AMD_MEM_ALLOC_LINES_MAX_NUM) {
        amd_dprintERROR("MEM DEBUGGER ERROR: too many allocation sites, max is %d.", AMD_MEM_ALLOC_LINES_MAX_NUM);
        AMD_EXIT(1);
    }
    for (i = 0; i < mem_alloc_lines_count; i++) {
        if (line == mem_alloc_lines[i].line) {
            if (amd_strncmp(file, mem_alloc_lines[i].file, AMD_MAX_LEN)) {
                break;
            }
        }
    }
    if (i < mem_alloc_lines_count) {
        struct Amd_Mem_Alloc_Buf mem_alloc_buf = {
            .buf = p,
            .size = size
        };
        amd_ada_appand(struct Amd_Mem_Alloc_Buf, mem_alloc_lines[i], mem_alloc_buf);
        mem_alloc_lines[i].size += size;
        mem_alloc_lines[i].allocations_over_all++;
        mem_alloc_lines[i].bytes_allocated_over_all += size;
    } else {
        if (i < AMD_MEM_ALLOC_LINES_MAX_NUM) {
            struct Amd_Mem_Alloc_Buf mem_alloc_buf = {
                .buf = p,
                .size = size
            };
            amd_ada_init_array(struct Amd_Mem_Alloc_Buf, mem_alloc_lines[i]);
            mem_alloc_lines[i].line = line;
            amd_strncpy(mem_alloc_lines[i].file, file, AMD_MAX_LEN);
            mem_alloc_lines[i].size = 0;
            mem_alloc_lines[i].allocations_over_all = 0;
            mem_alloc_lines[i].bytes_allocated_over_all = 0;
            mem_alloc_lines[i].freed = 0;

            amd_ada_appand(struct Amd_Mem_Alloc_Buf, mem_alloc_lines[i], mem_alloc_buf);
            mem_alloc_lines[i].size += size;
            mem_alloc_lines[i].bytes_allocated_over_all += size;
            mem_alloc_lines[i].allocations_over_all++;
            mem_alloc_lines_count++;
        }
    }
}

AMD_DEF size_t amd_debug_mem_consumption(void)
{
    size_t sum = 0;
    for (size_t i = 0; i < mem_alloc_lines_count; i++) {
        sum += mem_alloc_lines[i].size;
    }

    return sum;
}

AMD_DEF void amd_debug_mem_free(void *buf)
{
    if (buf == NULL) {
        return;
    }
    if (AMD_FAIL == amd_debug_mem_remove(buf)) {
        amd_dprintERROR("MEM ERROR: error occurred while trying to free buf: %p.", buf);
        AMD_EXIT(1);
    }
    AMD_FREE(buf);
}

AMD_DEF void *amd_debug_mem_malloc(size_t size, char *file, size_t line)
{
    void *p = AMD_MALLOC(size + AMD_MEM_OVER_ALLOC);
    if (NULL == p) {
        amd_dprintERROR("MEM ERROR: malloc returns NULL when trying to allocate %zu bytes at line %zu of file '%s'.", size, line, file);
        /* maybe print more debug info */
        AMD_EXIT(1);
    }
    for (size_t i = 0; i < size + AMD_MEM_OVER_ALLOC; i++) {
        ((uint8_t *)p)[i] = AMD_MEM_MAGIC_NUMBER + 1;
    }
    amd_debug_mem_add(p, size, file, line);

    return p;
}

AMD_DEF void amd_debug_mem_print(size_t min_allocs)
{
    amd_dprintINFO("%s", "Memory report:");
    printf("\33[A");
    size_t bytes_not_freed = 0;
    for (size_t i = 0; i < mem_alloc_lines_count; i++) {
        if (min_allocs <= mem_alloc_lines[i].allocations_over_all) {
            printf("%*.s%s:%zu\n", 7, "", mem_alloc_lines[i].file, mem_alloc_lines[i].line);
            printf("%*.s - Allocations: %zu\n", 7, "", mem_alloc_lines[i].allocations_over_all);
            printf("%*.s - Frees: %zu\n", 7, "", mem_alloc_lines[i].freed);
            printf("%*.s - Bytes allocated over all: %zu\n", 7, "", mem_alloc_lines[i].bytes_allocated_over_all);
            printf("%*.s - Bytes remained not freed: %zu\n", 7, "", mem_alloc_lines[i].size);
            bytes_not_freed += mem_alloc_lines[i].size;
        }
    }
    printf("%*.sTotal bytes remained not freed: %zu\n", 7, "", bytes_not_freed);
}

AMD_DEF void *amd_debug_mem_realloc(void *p, size_t size, char *file, size_t line)
{
    size_t i, j = 0, k, move;
    void *p2;
    if (p == NULL) {
        return amd_debug_mem_malloc(size, file, line);
    }
    for (i = 0; i < mem_alloc_lines_count; i++) {
        for (j = 0; j < mem_alloc_lines[i].length; j++) {
            if (mem_alloc_lines[i].elements[j].buf == p) {
                break;
            }
        }
        if (j < mem_alloc_lines[i].length) {
            break;
        }
    }
    if (i == mem_alloc_lines_count) {
        amd_dprintERROR("MEM DEBUGGER ERROR: tring to reallocate an unknown pointer %p in %s:%zu.", p, file, line);
        for (i = 0; i < mem_alloc_lines_count; i++) {
            for (j = 0; j < mem_alloc_lines[i].length; j++) {
                uint8_t *buf = mem_alloc_lines[i].elements[j].buf;
                for (k = 0; k < mem_alloc_lines[i].elements[j].size; k++) {
                    if (&buf[k] == (uint8_t *)p) {
                        amd_dprintERROR("Trying to reallocate pointer at %zu bytes (out of %zu) in to allocation made in %s:%zu.", k, mem_alloc_lines[i].elements[j].size, mem_alloc_lines[i].file, mem_alloc_lines[i].line);
                    }
                }
            }
        }
        AMD_EXIT(1);
    }
    move = mem_alloc_lines[i].elements[j].size;
    if (move > size) move = size;

    p2 = AMD_MALLOC(size + AMD_MEM_OVER_ALLOC);
    if (NULL == p2) {
        amd_dprintERROR("MEM ERROR: malloc returns NULL when trying to allocate %zu bytes at line %zu of file '%s'.", size, line, file);
        /* maybe print more debug info */
        AMD_EXIT(1);
    }
    for (i = 0; i < size + AMD_MEM_OVER_ALLOC; i++) {
        ((uint8_t *)p2)[i] = AMD_MEM_MAGIC_NUMBER + 1;
    }
    for (i = 0; i < move; i++) {
        ((uint8_t *)p2)[i] = ((uint8_t *)p)[i];
    }
    amd_debug_mem_add(p2, size, file, line);
    amd_debug_mem_remove(p);
    AMD_FREE(p);

    return p2;
}

AMD_DEF bool amd_debug_mem_remove(void *buf)
{
    size_t i, j, k;
    for (i = 0; i < mem_alloc_lines_count; i++) {
        for (j = 0; j < mem_alloc_lines[i].length; j++) {
            if (mem_alloc_lines[i].elements[j].buf == buf) {
                for (k = 0; k < AMD_MEM_OVER_ALLOC; k++) {
                    if (((uint8_t *)buf)[mem_alloc_lines[i].elements[j].size + k] != AMD_MEM_MAGIC_NUMBER) {
                        break;
                    }
                }
                if (k < AMD_MEM_OVER_ALLOC) {
                    amd_dprintERROR("MEM ERROR: overshoot at line %zu in file '%s'.", mem_alloc_lines[i].line, mem_alloc_lines[i].file);
                }
                mem_alloc_lines[i].size -= mem_alloc_lines[i].elements[j].size;
                amd_ada_remove_unordered(struct Amd_Mem_Alloc_Buf, mem_alloc_lines[i], j);
                mem_alloc_lines[i].freed++;
                return AMD_SUCCESS;
            }
        }
    }
    return AMD_FAIL;
}

AMD_DEF void amd_debug_mem_reset(void)
{
    for (size_t i = 0; i < mem_alloc_lines_count; i++) {
        AMD_FREE(mem_alloc_lines[i].elements);
        mem_alloc_lines[i].elements = NULL;
        mem_alloc_lines[i].capacity = 0;
        mem_alloc_lines[i].length = 0;
        mem_alloc_lines[i].line = 0;
        mem_alloc_lines[i].file[0] = '\0';
        mem_alloc_lines[i].size = 0;
        mem_alloc_lines[i].allocations_over_all = 0;
        mem_alloc_lines[i].bytes_allocated_over_all = 0;
        mem_alloc_lines[i].freed = 0;
    }
    mem_alloc_lines_count = 0;
}

AMD_DEF int amd_strncmp(const char *s1, const char *s2, const size_t N)
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

AMD_DEF int amd_strncpy(char * const s1, const char * const s2, const size_t N)
{
    if (N == 0) return 0;

    size_t n = amd_min(N, (size_t)AMD_MAX_LEN - 1);

    size_t i;
    for (i = 0; i < n && s2[i] != '\0'; i++) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';

    return (int)i;
}

#endif /*ALMOG_MEMORY_DEBUG_IMPLEMENTATION*/

#ifdef AMD_MEMORY_DEBUG
    #define malloc(n) amd_debug_mem_malloc(n, __FILE__, __LINE__)
    #define realloc(n, m) amd_debug_mem_realloc(n, m, __FILE__, __LINE__)
    #define free(n) amd_debug_mem_free(n)
#endif
