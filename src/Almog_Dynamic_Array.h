/**
 * @file
 * @brief Header-only C macros that implement a simple dynamic array.
 *
 * This header provides a minimal, macro-based dynamic array for POD-like types.
 * The array "header" is a user-defined struct with three fields:
 *   - size_t length;   current number of elements
 *   - size_t capacity; allocated capacity (in elements)
 *   - T*     elements; pointer to contiguous storage of elements (type T)
 *
 * How to use:
 *   1) Define a header struct with length/capacity/elements fields.
 *   2) Initialize it with ada_init_array(T, header).
 *   3) Modify it with ada_appand (append), ada_insert, remove variants, etc.
 *   4) When done, free(header.elements) (or your custom deallocator).
 *
 * Customization:
 *   - Define ADA_MALLOC, ADA_REALLOC, and ADA_ASSERT before including this
 *     header to override allocation and assertion behavior.
 *
 * Complexity (n = number of elements):
 *   - Append: amortized O(1)
 *   - Ordered insert/remove: O(n)
 *   - Unordered insert/remove: O(1)
 *
 * Notes and limitations:
 *   - These are macros; arguments may be evaluated multiple times. Pass only
 *     simple lvalues (no side effects).
 *   - Index checks rely on ADA_ASSERT; with NDEBUG they may be compiled out.
 *   - ada_resize exits the process (exit(1)) if reallocation fails.
 *   - ada_insert reads header.elements[header.length - 1] internally; inserting
 *     into an empty array via ada_insert is undefined behavior. Use
 *     ada_appand or ada_insert_unordered for that case.
 *   - No automatic shrinking; you may call ada_resize manually.
 *
 * Example:
 *   typedef struct {
 *     size_t length;
 *     size_t capacity;
 *     int* elements;
 *   } ada_int_array;
 *
 *   ada_int_array arr;
 *   ada_init_array(int, arr);
 *   ada_appand(int, arr, 42);
 *   ada_insert(int, arr, 7, 0); // requires arr.length > 0
 *   ada_remove(int, arr, 1);
 *   free(arr.elements);
 */

#ifndef ALMOG_DYNAMIC_ARRAY_H_
#define ALMOG_DYNAMIC_ARRAY_H_


/**
 * @def ADA_INIT_CAPACITY
 * @brief Default initial capacity used by ada_init_array.
 *
 * You may override this by defining ADA_INIT_CAPACITY before including this file.
 */
#ifndef ADA_INIT_CAPACITY
#define ADA_INIT_CAPACITY 10
#endif /*ADA_INIT_CAPACITY*/

/**
 * @def ADA_MALLOC
 * @brief Allocation function used by this header (defaults to malloc).
 *
 * Define ADA_MALLOC to a compatible allocator before including this file to
 * override the default.
 */
#ifndef ADA_MALLOC
#include <stdlib.h>
#define ADA_MALLOC malloc
#endif /*ADA_MALLOC*/

#ifndef ADA_EXIT
#include <stdlib.h>
#define ADA_EXIT exit
#endif /*ADA_EXIT*/

/**
 * @def ADA_REALLOC
 * @brief Reallocation function used by this header (defaults to realloc).
 *
 * Define ADA_REALLOC to a compatible reallocator before including this file to
 * override the default.
 */
#ifndef ADA_REALLOC
#include <stdlib.h>
#define ADA_REALLOC realloc
#endif /*ADA_REALLOC*/

/**
 * @def ADA_ASSERT
 * @brief Assertion macro used by this header (defaults to assert).
 *
 * Define ADA_ASSERT before including this file to override. When NDEBUG is
 * defined, standard assert() is disabled.
 */
#ifndef ADA_ASSERT
#include <assert.h>
#define ADA_ASSERT assert
#endif /*ADA_ASSERT*/

/* typedef struct {
    size_t length;
    size_t capacity;
    int* elements;
} ada_int_array; */

/**
 * @def ada_init_array(type, header)
 * @brief Initialize an array header and allocate its initial storage.
 *
 * @param type   Element type stored in the array (e.g., int).
 * @param header Lvalue of the header struct containing fields:
 *               length, capacity, and elements.
 *
 * @pre header is a modifiable lvalue; header.elements is uninitialized or
 *      ignored and will be overwritten.
 * @post header.length == 0, header.capacity == INIT_CAPACITY,
 *       header.elements != NULL (or ADA_ASSERT fails).
 *
 * @note Allocation uses ADA_MALLOC and is checked via ADA_ASSERT.
 */
#define ada_init_array(type, header) do {                                       \
        header.capacity = ADA_INIT_CAPACITY;                                        \
        header.length = 0;                                                      \
        header.elements = (type *)ADA_MALLOC(sizeof(type) * header.capacity);   \
        ADA_ASSERT(header.elements != NULL);                                    \
    } while (0)

    /**
 * @def ada_resize(type, header, new_capacity)
 * @brief Resize the underlying storage to hold new_capacity elements.
 *
 * @param type         Element type stored in the array.
 * @param header       Lvalue of the header struct.
 * @param new_capacity New capacity in number of elements.
 *
 * @pre new_capacity >= header.length (otherwise elements beyond new_capacity
 *      are lost and length will not be adjusted).
 * @post header.capacity == new_capacity and header.elements points to a block
 *       large enough for new_capacity elements.
 *
 * @warning On allocation failure, this macro calls ADA_EXIT(1).
 * @note Reallocation uses ADA_REALLOC and is also checked via ADA_ASSERT.
 */
#define ada_resize(type, header, new_capacity) do {                                                         \
        type *ada_temp_pointer = (type *)ADA_REALLOC((void *)(header.elements), new_capacity*sizeof(type)); \
        if (ada_temp_pointer == NULL) {                                                                     \
            ADA_EXIT(1);                                                                                        \
        }                                                                                                   \
        header.elements = ada_temp_pointer;                                                                 \
        ADA_ASSERT(header.elements != NULL);                                                                \
        header.capacity = new_capacity;                                                                     \
    } while (0)

/**
 * @def ada_appand(type, header, value)
 * @brief Append a value to the end of the array, growing if necessary.
 *
 * @param type   Element type stored in the array.
 * @param header Lvalue of the header struct.
 * @param value  Value to append.
 *
 * @post header.length is incremented by 1; the last element equals value.
 *
 * @note Growth factor is (int)(header.capacity * 1.5). Because of truncation,
 *       very small capacities may not grow (e.g., from 1 to 1). With the
 *       default INIT_CAPACITY=10 this is typically not an issue unless you
 *       manually shrink capacity. Ensure growth always increases capacity by
 *       at least 1 if you customize this macro.
 */
#define ada_appand(type, header, value) do {                                            \
        if (header.length >= header.capacity) {                                         \
            ada_resize(type, header, (int)(header.capacity + header.capacity/2 + 1));   \
        }                                                                               \
        header.elements[header.length] = value;                                         \
        header.length++;                                                                \
    } while (0)

/**
 * @def ada_insert(type, header, value, index)
 * @brief Insert value at position index, preserving order (O(n)).
 *
 * @param type   Element type stored in the array.
 * @param header Lvalue of the header struct.
 * @param value  Value to insert.
 * @param index  Destination index in the range [0, header.length].
 *
 * @pre 0 <= index <= header.length.
 * @pre header.length > 0 if index == header.length (this macro reads the last
 *      element internally). For inserting into an empty array, use
 *      ada_appand or ada_insert_unordered.
 * @post Element is inserted at index; subsequent elements are shifted right;
 *       header.length is incremented by 1.
 *
 * @note This macro asserts index is non-negative and an integer value using
 *       ADA_ASSERT. No explicit upper-bound assert is performed.
 */
#define ada_insert(type, header, value, index) do {                                                             \
    ADA_ASSERT((int)(index) >= 0);                                                                              \
    ADA_ASSERT((float)(index) - (int)(index) == 0);                                                             \
    ada_appand(type, header, header.elements[header.length-1]);                                                 \
    for (int ada_for_loop_index = header.length-2; ada_for_loop_index > (int)(index); ada_for_loop_index--) {   \
        header.elements[ada_for_loop_index] = header.elements [ada_for_loop_index-1];                           \
    }                                                                                                           \
    header.elements[(index)] = value;                                                                           \
} while (0)


/**
 * @def ada_insert_unordered(type, header, value, index)
 * @brief Insert value at index without preserving order (O(1) amortized).
 *
 * If index == header.length, this behaves like an append. Otherwise, the
 * current element at index is moved to the end, and value is written at index.
 *
 * @param type   Element type stored in the array.
 * @param header Lvalue of the header struct.
 * @param value  Value to insert.
 * @param index  Index in the range [0, header.length].
 *
 * @pre 0 <= index <= header.length.
 * @post header.length is incremented by 1; array order is not preserved.
 */
#define ada_insert_unordered(type, header, value, index) do {   \
    ADA_ASSERT((int)(index) >= 0);                              \
    ADA_ASSERT((float)(index) - (int)(index) == 0);             \
    if ((size_t)(index) == header.length) {                     \
        ada_appand(type, header, value);                        \
    } else {                                                    \
        ada_appand(type, header, header.elements[(index)]);     \
        header.elements[(index)] = value;                       \
    }                                                           \
} while (0)

/**
 * @def ada_remove(type, header, index)
 * @brief Remove element at index, preserving order (O(n)).
 *
 * @param type   Element type stored in the array.
 * @param header Lvalue of the header struct.
 * @param index  Index in the range [0, header.length - 1].
 *
 * @pre 0 <= index < header.length.
 * @post header.length is decremented by 1; subsequent elements are shifted
 *       left by one position. The element beyond the new length is left
 *       uninitialized.
 */
#define ada_remove(type, header, index) do {                                                                \
    ADA_ASSERT((int)(index) >= 0);                                                                          \
    ADA_ASSERT((float)(index) - (int)(index) == 0);                                                         \
    for (size_t ada_for_loop_index = (index); ada_for_loop_index < header.length-1; ada_for_loop_index++) { \
        header.elements[ada_for_loop_index] = header.elements[ada_for_loop_index+1];                        \
    }                                                                                                       \
    header.length--;                                                                                        \
} while (0)

/**
 * @def ada_remove_unordered(type, header, index)
 * @brief Remove element at index by moving the last element into its place
 *        (O(1)); order is not preserved.
 *
 * @param type   Element type stored in the array.
 * @param header Lvalue of the header struct.
 * @param index  Index in the range [0, header.length - 1].
 *
 * @pre 0 <= index < header.length and header.length > 0.
 * @post header.length is decremented by 1; array order is not preserved.
 */
#define ada_remove_unordered(type, header, index) do {          \
    ADA_ASSERT((int)(index) >= 0);                              \
    ADA_ASSERT((float)(index) - (int)(index) == 0);             \
    header.elements[index] = header.elements[header.length-1];  \
    header.length--;                                            \
} while (0)


#endif /*ALMOG_DYNAMIC_ARRAY_H_*/
