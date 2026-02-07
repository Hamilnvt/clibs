/* Dynamic Arrays in C (inspired by Tsoding https://github.com/tsoding).

A dynamic array holding elements of type T is a struct that must have at least these fields:
struct DA {
    T *items;            - array of elements of type T
    size_t count;        - current number of elements
    size_t capacity;     - current capacity
};
*/

#ifndef INCLUDE_DA_H
#define INCLUDE_DA_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define DA_DEFAULT_CAP 32

/*================== Get & Other Utilities =====================*/

#define da_bounds_check(da, i) \
    assert((size_t)(i) < (da).count && "Index must be less than the array count.")

#define da_get(da, i) \
    (da_bounds_check((da), (i)), (da).items[i])

#define da_get_ptr(da, i) \
    (da_bounds_check((da), (i)), &(da).items[i])

#define da_is_empty(da) ((da)->count == 0)

#define da_fit(da)                                                                     \
    do {                                                                               \
        if ((da)->count > 0) {                                                         \
            (da)->capacity = (da)->count;                                              \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof((da)->items[0])); \
            assert((da)->items != NULL && "Buy more RAM");                             \
        }                                                                              \
    } while (0)

#define da_sort(da_ptr, cmp) \
    qsort((da_ptr)->items, (da_ptr)->count, sizeof(*(da_ptr)->items), (cmp))

#define da_binary_search(da, key_ptr, cmp) \
    bsearch((key_ptr), (da).items, (da).count, sizeof(*(da).items), (cmp))

/*================== Insert =====================*/

#define da_push(da, item)                                                             \
    do {                                                                              \
        if ((da)->count >= (da)->capacity) {                                          \
            (da)->capacity = (da)->capacity == 0 ? DA_DEFAULT_CAP : (da)->capacity*2; \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));  \
            assert((da)->items != NULL && "Buy more RAM");                            \
        }                                                                             \
        (da)->items[(da)->count++] = (item);                                          \
    } while (0)

#define da_insert(da, item, i)                                                               \
    do {                                                                                     \
        if ((da)->count >= (da)->capacity) {                                                 \
            (da)->capacity = (da)->capacity == 0 ? DA_DEFAULT_CAP : (da)->capacity*2;        \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));         \
            assert((da)->items != NULL && "Buy more RAM");                                   \
        }                                                                                    \
        if ((size_t)(i) == (da)->count) {                                                    \
            (da)->items[(da)->count++] = (item);                                             \
        } else {                                                                             \
            assert((size_t)(i) < (da)->count && "Index must be less than the array count."); \
            memmove((da)->items+i+1, (da)->items+i, ((da)->count-i)*sizeof(*(da)->items));   \
            (da)->items[i] = (item);                                                         \
            (da)->count++;                                                                   \
        }                                                                                    \
    } while (0)

#define da_push_first(da, item) da_insert((da), (item), 0)

#define da_push_to_sorted(da_ptr, item, cmp)             \
    do {                                                 \
        bool found = false;                              \
        __typeof__((da_ptr)->items[0]) tmp = (item);     \
        for (size_t i = 0; i < (da_ptr)->count; i++) {   \
            if ((cmp)(&(da_ptr)->items[i], &tmp) >= 0) { \
                da_insert((da_ptr), (item), i);          \
                found = true;                            \
                break;                                   \
            }                                            \
        }                                                \
        if (!found) da_push((da_ptr), (item));           \
    } while (0)

#define da_push_many(da, new_items, new_items_count)                                            \
    do {                                                                                        \
        if ((da)->count + (new_items_count) > (da)->capacity) {                                 \
            if ((da)->capacity == 0) (da)->capacity = DA_DEFAULT_CAP;                           \
            while ((da)->count + (new_items_count) > (da)->capacity) (da)->capacity *= 2;       \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));            \
            assert((da)->items != NULL && "Buy more RAM");                                      \
        }                                                                                       \
        memcpy((da)->items + (da)->count, (new_items), (new_items_count)*sizeof(*(da)->items)); \
        (da)->count += (new_items_count);                                                       \
    } while (0)

/*================== Remove =====================*/

#define da_pop(da) \
    (assert((da)->count > 0 && "DA ERROR: underflow"), (da)->items[--(da)->count])

#define da_remove(da, i)                                                                     \
    do {                                                                                     \
        assert((da)->count > 0 && "DA ERROR: underflow");                                    \
        assert((size_t)(i) < (da)->count && "Index must be less than the array count.");     \
        if ((size_t)(i) < (da)->count - 1)                                                   \
            memmove((da)->items+i, (da)->items+i+1, ((da)->count-i-1)*sizeof(*(da)->items)); \
        (da)->count--;                                                                       \
    } while (0)

#define da_remove_first(da) da_remove((da), 0)

/*================== Iterators =====================*/

#define da_for(da, i) for (size_t i = 0; i < (da).count; i++)

#define da_foreach(da, Type, item) \
    for (Type *item = (da).items; item != NULL && item < (da).items+(da).count; item++)

#define da_enumerate(da, Type, i, item) \
    for (size_t i = 0; i < (da).count; i++) \
        for (Type *item = &(da).items[i]; item; item = NULL)

/*================== Cleanup =====================*/

// NOTE: does not free
#define da_clear(da) \
    (da)->count = 0;

#define da_free(da)         \
    do {                    \
        free((da)->items);  \
        (da)->items = NULL; \
        (da)->count = 0;    \
        (da)->capacity = 0; \
    } while (0)

#endif // INCLUDE_DA_H
