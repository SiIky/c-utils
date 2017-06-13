/*
 * A vector type inspired by
 *  * Rust's `Vec` type
 *  * [stb](https://github.com/nothings/stb)
 *  * [sort](https://github.com/swenson/sort)
 *
 * # Usage
 *
 * ```c
 * #include <assert.h>
 *
 * #define VEC_DATA_TYPE size_t
 *
 * // Optionally define a prefix (defaults to vec_)
 * #define VEC_PREFIX your_prefix_
 *
 * // Optionally define NDEBUG to disable asserts inside vec.h
 * #define NDEBUG
 *
 * // Optionally define VEC_STATIC to mark definitions as static
 * // Probably will raise `unused-function` warnings
 * #defined VEC_STATIC
 *
 * #include <vec.h>
 *
 * int main (void)
 * {
 *     struct your_prefix_Vec vec = your_prefix_new();
 *
 *     for (size_t i = 0; i < 10; i++)
 *         your_prefix_push(&vec, i);
 *
 *     for (size_t i = 0; i < 10; i++)
 *         assert(vec.ptr[i] == i);
 *
 *     your_prefix_free(&vec);
 *
 *     assert(vec.ptr == NULL);
 *
 *     return 0;
 * }
 * ```
 *
 * # TODO
 *
 * - [X] Allow more than one implementation for different types
 * - [ ] More options
 */

#include <assert.h>  /* assert() */
#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <stdlib.h>  /* calloc(), realloc() */
#include <string.h>  /* memcpy() */

/*
 * Type of data for the Vec to hold
 */
#ifndef VEC_DATA_TYPE
# error "Must declare VEC_DATA_TYPE"
#endif /* VEC_DATA_TYPE */

/*
 * If no prefix was defined, default to `vec_`
 */
#ifndef VEC_PREFIX
# define VEC_PREFIX vec_
#endif /* VEC_PREFIX */

/*
 * Magic from `sort.h`
 */
#define VEC_CONCAT(A, B)    A ## B
#define VEC_MAKE_STR1(A, B) VEC_CONCAT(A, B)
#define VEC_MAKE_STR(A)     VEC_MAKE_STR1(VEC_PREFIX, A)

#define VEC_VEC VEC_MAKE_STR(Vec)
/**=========================================================
 * @brief The vector type.
 */
struct VEC_VEC {
    /** Pointer to actual data. */
    VEC_DATA_TYPE * ptr;
    /** Number of elements. */
    size_t length;
    /** Number of elements the Vec can hold currently. */
    size_t capacity;
};

# ifdef VEC_STATIC
#  undef VEC_STATIC
#  define VEC_STATIC static
# else /* VEC_STATIC */
#  undef VEC_STATIC
#  define VEC_STATIC
# endif /* VEC_STATIC */

/*==========================================================
 * Function names
==========================================================*/
#define VEC_APPEND             VEC_MAKE_STR(append)
#define VEC_AS_MUT_SLICE       VEC_MAKE_STR(as_mut_slice)
#define VEC_AS_SLICE           VEC_MAKE_STR(as_slice)
#define VEC_CAPACITY           VEC_MAKE_STR(capacity)
#define VEC_FREE               VEC_MAKE_STR(free)
#define VEC_FROM_RAW_PARTS     VEC_MAKE_STR(from_raw_parts)
#define VEC_GET_NTH            VEC_MAKE_STR(get_nth)
#define VEC_INSERT             VEC_MAKE_STR(insert)
#define VEC_IS_EMPTY           VEC_MAKE_STR(is_empty)
#define VEC_LEN                VEC_MAKE_STR(len)
#define VEC_NEW                VEC_MAKE_STR(new)
#define VEC_POP                VEC_MAKE_STR(pop)
#define VEC_PUSH               VEC_MAKE_STR(push)
#define VEC_REMOVE             VEC_MAKE_STR(remove)
#define VEC_RESERVE            VEC_MAKE_STR(reserve)
#define VEC_RETAIN             VEC_MAKE_STR(retain)
#define VEC_SET_LEN            VEC_MAKE_STR(set_len)
#define VEC_SET_NTH            VEC_MAKE_STR(set_nth)
#define VEC_SHRINK_TO_FIT      VEC_MAKE_STR(shrink_to_fit)
#define VEC_SPLIT_OFF          VEC_MAKE_STR(split_off)
#define VEC_SWAP_REMOVE        VEC_MAKE_STR(swap_remove)
#define VEC_TRUNCATE           VEC_MAKE_STR(truncate)
#define VEC_WITH_CAPACITY      VEC_MAKE_STR(with_capacity)
#define _VEC_INCREASE_CAPACITY VEC_MAKE_STR(_increase_capacity)

/*==========================================================
 * Function prototypes
==========================================================*/
VEC_STATIC VEC_DATA_TYPE * VEC_AS_MUT_SLICE (struct VEC_VEC * self);
VEC_STATIC VEC_DATA_TYPE VEC_GET_NTH (const struct VEC_VEC * self, size_t nth);
VEC_STATIC VEC_DATA_TYPE VEC_POP (struct VEC_VEC * self);
VEC_STATIC VEC_DATA_TYPE VEC_REMOVE (struct VEC_VEC * self, size_t index);
VEC_STATIC VEC_DATA_TYPE VEC_SWAP_REMOVE (struct VEC_VEC * self, size_t index);
VEC_STATIC bool VEC_IS_EMPTY (const struct VEC_VEC * self);
VEC_STATIC const VEC_DATA_TYPE * VEC_AS_SLICE (const struct VEC_VEC * self);
VEC_STATIC size_t VEC_CAPACITY (const struct VEC_VEC * self);
VEC_STATIC size_t VEC_LEN (const struct VEC_VEC * self);
VEC_STATIC struct VEC_VEC VEC_FROM_RAW_PARTS (VEC_DATA_TYPE * ptr, size_t length, size_t capacity);
VEC_STATIC struct VEC_VEC VEC_NEW (void);
VEC_STATIC struct VEC_VEC VEC_SPLIT_OFF (struct VEC_VEC * self, size_t at);
VEC_STATIC struct VEC_VEC VEC_WITH_CAPACITY (size_t capacity);
VEC_STATIC void VEC_APPEND (struct VEC_VEC * self, struct VEC_VEC * other);
VEC_STATIC void VEC_FREE (struct VEC_VEC * self);
VEC_STATIC void VEC_INSERT (struct VEC_VEC * self, size_t index, VEC_DATA_TYPE element);
VEC_STATIC void VEC_PUSH (struct VEC_VEC * self, VEC_DATA_TYPE value);
VEC_STATIC void VEC_RESERVE (struct VEC_VEC * self, size_t additional);
VEC_STATIC void VEC_RETAIN (struct VEC_VEC * self, bool (* f) (VEC_DATA_TYPE *));
VEC_STATIC void VEC_SET_LEN (struct VEC_VEC * self, size_t len);
VEC_STATIC void VEC_SET_NTH (struct VEC_VEC * self, size_t nth, VEC_DATA_TYPE element);
VEC_STATIC void VEC_SHRINK_TO_FIT (struct VEC_VEC * self);
VEC_STATIC void VEC_TRUNCATE (struct VEC_VEC * self, size_t len);

/*==========================================================
 * Function definitions
==========================================================*/

/**=========================================================
 * @brief Checks if self needs more memory
 * @param self The vec to check
 */
VEC_STATIC void _VEC_INCREASE_CAPACITY (struct VEC_VEC * self)
{
    assert(self != NULL);
    if (self->length == self->capacity)
        VEC_RESERVE(self, (self->capacity >> 1) + 1);
}

/**=========================================================
 * @brief Free a vec type
 * @param self The vec to free
 */
VEC_STATIC void VEC_FREE (struct VEC_VEC * self)
{
    assert(self != NULL);

    if (self->ptr != NULL) {
        free(self->ptr);
        self->ptr = NULL;
    }

    self->capacity = 0;
    self->length = 0;
}

/**=========================================================
 * @brief Creates a new empty vec
 * @returns The new vec
 */
VEC_STATIC struct VEC_VEC VEC_NEW (void)
{
    return VEC_FROM_RAW_PARTS(NULL, 0, 0);
}

/**=========================================================
 * @brief Creates a new vec and allocates memory
 * @param capacity Number of elements to allocate
 * @returns The new vec
 */
VEC_STATIC struct VEC_VEC VEC_WITH_CAPACITY (size_t capacity)
{
    VEC_DATA_TYPE * data = calloc(capacity, sizeof(VEC_DATA_TYPE));
    capacity = (data != NULL) ? capacity : 0;
    return VEC_FROM_RAW_PARTS(data, 0, capacity);
}

/**=========================================================
 * @brief Creates a vec from seperate components
 * @param ptr A pointer to allocd memory
 * @param length Number of elements in ptr
 * @param capacity Number of elements vec can hold
 * @returns A new vec pointing to ptr, with length and capacity
 */
VEC_STATIC struct VEC_VEC VEC_FROM_RAW_PARTS (VEC_DATA_TYPE * ptr, size_t length, size_t capacity)
{
    return (struct VEC_VEC) {
        .ptr = ptr,
        .length = length,
        .capacity = capacity,
    };
}

/**=========================================================
 * @brief Calculates the capacity of a vec
 * @param self The vec to calculate capacity
 * @returns Capacity of self
 */
VEC_STATIC size_t VEC_CAPACITY (const struct VEC_VEC * self)
{
    assert(self != NULL);
    return self->capacity;
}

/**=========================================================
 * @brief Reserve memory for at least additional elements
 * @param self The vec
 * @param additional Number of additional elements
 */
VEC_STATIC void VEC_RESERVE (struct VEC_VEC * self, size_t additional)
{
    assert(self != NULL);

    size_t sum = self->length + additional;

    if (self->capacity >= sum)
        return;

    self->capacity = sum;
    self->ptr = realloc(self->ptr, sum * sizeof(VEC_DATA_TYPE));
}

/**=========================================================
 * @brief Shrink the capacity of the vec to length
 * @param self The vec
 */
VEC_STATIC void VEC_SHRINK_TO_FIT (struct VEC_VEC * self)
{
    assert(self != NULL);
    self->capacity = self->length;
    self->ptr = realloc(self->ptr, self->length * sizeof(VEC_DATA_TYPE));
}

/**=========================================================
 * @brief Shorten the vector, keeping the first len elements
 * @param self The vec
 * @param len New length
 */
VEC_STATIC void VEC_TRUNCATE (struct VEC_VEC * self, size_t len)
{
    assert(self != NULL);
    if (self->length > len)
        self->length = len;
}

/**=========================================================
 * @brief Returns a const pointer to the allocated memory
 * @param self The vec
 * @returns `const` pointer to allocated memory
 */
VEC_STATIC const VEC_DATA_TYPE * VEC_AS_SLICE (const struct VEC_VEC * self)
{
    assert(self != NULL);
    return (const VEC_DATA_TYPE *) self->ptr;
}

/**=========================================================
 * @brief Like vec_as_slice, but not `const`
 * @param self The vec
 * @returns Pointer to allocated memory
 */
VEC_STATIC VEC_DATA_TYPE * VEC_AS_MUT_SLICE (struct VEC_VEC * self)
{
    assert(self != NULL);
    return self->ptr;
}

/**=========================================================
 * @brief Sets the length of a vec, without checks
 * @param self The vec
 * @param len The new length
 */
VEC_STATIC void VEC_SET_LEN (struct VEC_VEC * self, size_t len)
{
    assert(self != NULL);
    self->length = len;
}

/**=========================================================
 * @brief Removes an element, replacing it with the last element
 * @param self The vec
 * @param index Index of the element to be removed
 * @returns The removed element
 */
VEC_STATIC VEC_DATA_TYPE VEC_SWAP_REMOVE (struct VEC_VEC * self, size_t index)
{
    assert(self != NULL);
    assert(index < self->length);
    assert(self->length > 0);

    VEC_DATA_TYPE ret = self->ptr[index];
    self->ptr[index] = VEC_POP(self);

    return ret;
}

/**=========================================================
 * @brief Inserts an element at index, shifting every element after it to the right
 * @param self The vec
 * @param index Where the element will be inserted
 * @param element Element to be inserted
 */
VEC_STATIC void VEC_INSERT (struct VEC_VEC * self, size_t index, VEC_DATA_TYPE element)
{
    assert(self != NULL);
    assert(index < self->length);

    _VEC_INCREASE_CAPACITY(self);
    if (self->length == self->capacity)
        return;

    for (size_t i = self->length; i >= index; i--)
        self->ptr[i] = self->ptr[i-1];

    self->length++;
    self->ptr[index] = element;
}

/**=========================================================
 * @brief Removes an element, shifting every element after it to the left
 * @param self The vec
 * @param index Index of the element to remove
 * @returns Removed element
 */
VEC_STATIC VEC_DATA_TYPE VEC_REMOVE (struct VEC_VEC * self, size_t index)
{
    assert(self != NULL);
    assert(index < self->length);

    VEC_DATA_TYPE ret = self->ptr[index];

    for (size_t i = index + 1; i < self->length; i++)
        self->ptr[i - 1] = self->ptr[i];

    self->length--;

    return ret;
}

/**=========================================================
 * @brief Keep every element that satisfies a predicate
 * @param self The vec
 * @param f Predicate
 */
VEC_STATIC void VEC_RETAIN (struct VEC_VEC * self, bool (* f) (VEC_DATA_TYPE *))
{
    assert(self != NULL);
    assert(f != NULL);

    size_t w = 0;

    for (size_t r = 0; r < self->length; r++) {
        if (f(self->ptr + r)) {
            self->ptr[w] = self->ptr[r];
            w++;
        }
    }

    self->length = w;
}

/**=========================================================
 * @brief Insert an element at the end
 * @param self The vec
 * @param value Element to be pushed
 */
VEC_STATIC void VEC_PUSH (struct VEC_VEC * self, VEC_DATA_TYPE value)
{
    assert(self != NULL);

    _VEC_INCREASE_CAPACITY(self);
    if (self->length == self->capacity)
        return;

    self->ptr[self->length] = value;
    self->length++;
}

/**=========================================================
 * @brief Remove the last element
 * @param self The vec
 * @returns The removed element
 */
VEC_STATIC VEC_DATA_TYPE VEC_POP (struct VEC_VEC * self)
{
    assert(self != NULL);
    assert(self->length > 0);
    self->length--;
    return self->ptr[self->length];
}

/**=========================================================
 * @brief Appends a vec to self vec and sets the length of other to 0
 * @param self The vec
 * @param other The other vec
 */
VEC_STATIC void VEC_APPEND (struct VEC_VEC * self, struct VEC_VEC * other)
{
    assert(self != NULL);
    assert(other != NULL);

    size_t sum = self->length + other->length;
    VEC_RESERVE(self, sum);

    if (self->capacity < sum)
        return;

    for (size_t r = 0; r < other->length; r++) {
        self->ptr[self->length] = other->ptr[r];
        self->length++;
    }

    other->length = 0;
}

/**=========================================================
 * @brief Calculates the length of a vec
 * @param self The vec
 * @returns The length of self
 */
VEC_STATIC size_t VEC_LEN (const struct VEC_VEC * self)
{
    assert(self != NULL);
    return self->length;
}

/**=========================================================
 * @brief Checks if a vec is empty
 * @param self The vec
 * @returns `true` if `length == 0`, and `false` otherwise
 */
VEC_STATIC bool VEC_IS_EMPTY (const struct VEC_VEC * self)
{
    assert(self != NULL);
    return self->length == 0;
}

/**=========================================================
 * @brief Splits a vec in two, keeping the first `at - 1` in self, and the rest in the other
 * @param self The vec
 * @param at Where to split
 * @returns The new vec
 */
// FIXME
VEC_STATIC struct VEC_VEC VEC_SPLIT_OFF (struct VEC_VEC * self, size_t at)
{
    assert(self != NULL);
    assert(at < self->length);

    struct VEC_VEC ret = VEC_WITH_CAPACITY(self->length - at);

    memcpy(ret.ptr, self->ptr + at, ret.capacity);

    ret.length = self->capacity;

    return ret;
}

/**=========================================================
 * @brief Get the element at the nth index
 * @param self The vec
 * @param nth The index
 * @returns The element in index nth
 */
VEC_STATIC VEC_DATA_TYPE VEC_GET_NTH (const struct VEC_VEC * self, size_t nth)
{
    assert(self != NULL);
    assert(nth < self->length);
    return self->ptr[nth];
}

/**=========================================================
 * @brief Set the element at the nth index to element
 * @param self The vec
 * @param nth The index
 * @param element The new element
 */
VEC_STATIC void VEC_SET_NTH (struct VEC_VEC * self, size_t nth, VEC_DATA_TYPE element)
{
    assert(self != NULL);
    assert(nth < self->length);
    self->ptr[nth] = element;
}

/*==========================================================
 * Clean up
==========================================================*/

/*
 * Functions
 */
#undef VEC_APPEND
#undef VEC_AS_MUT_SLICE
#undef VEC_AS_SLICE
#undef VEC_CAPACITY
#undef VEC_FREE
#undef VEC_FROM_RAW_PARTS
#undef VEC_GET_NTH
#undef VEC_INSERT
#undef VEC_IS_EMPTY
#undef VEC_LEN
#undef VEC_NEW
#undef VEC_POP
#undef VEC_PUSH
#undef VEC_REMOVE
#undef VEC_RESERVE
#undef VEC_RETAIN
#undef VEC_SET_LEN
#undef VEC_SET_NTH
#undef VEC_SHRINK_TO_FIT
#undef VEC_SPLIT_OFF
#undef VEC_SWAP_REMOVE
#undef VEC_TRUNCATE
#undef VEC_WITH_CAPACITY
#undef _VEC_INCREASE_CAPACITY

/*
 * Other
 */
#undef VEC_DATA_TYPE
#undef VEC_PREFIX
#undef VEC_STATIC
#undef VEC_VEC

/*==========================================================
 * License
 *==========================================================
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */
