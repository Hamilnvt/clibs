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

// TODO:
// - same stuff but many
// - find
// - better realloc (expand when full at 3/4, and maybe shrink when full at < 3/4)
// - scartare il risultato nelle operazioni di remove
// - make it a header only library (vedi nob)

#define DA_DEFAULT_CAP 32

// TODO: always true for size_t i
// assert((i) >= 0 && "Index must be positive.");
#define da_bounds_check(da, i)                                                          \
    do {                                                                                \
        assert((size_t)(i) < (da).count && "Index must be less than the array count."); \
    } while (0)

#define da_get(da, i, item)         \
    do {                            \
        da_bounds_check((da), (i)); \
        (item) = (da).items[i];     \
    } while (0)

#define da_get_ptr(da, i, item)     \
    do {                            \
        da_bounds_check((da), (i)); \
        (item) = &(da).items[i];    \
    } while (0)

#define da_fit(da)                                                                     \
    do {                                                                               \
        if ((da)->count > 0) {                                                         \
            (da)->capacity = (da)->count;                                              \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof((da)->items[0])); \
            assert((da)->items != NULL && "Buy more RAM");                             \
        }                                                                              \
    } while (0)

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
            (da)->items = reallocarray((da)->items, (da)->capacity, sizeof(*(da)->items));   \
            assert((da)->items != NULL && "Buy more RAM");                                   \
        }                                                                                    \
        if ((size_t)(i) == (da)->count) {                                                    \
            (da)->items[(da)->count++] = (item);                                             \
        } else {                                                                             \
            assert((i) >= 0 && "Index must be positive.");                                   \
            assert((size_t)(i) < (da)->count && "Index must be less than the array count."); \
            memmove((da)->items+i+1, (da)->items+i, ((da)->count-i)*sizeof(*(da)->items));   \
            (da)->items[i] = (item);                                                         \
            (da)->count++;                                                                   \
        }                                                                                    \
    } while (0)

#define da_push_front(da, item) da_insert((da), (item), 0)

#define da_push_many(da, new_items, new_items_count)                                            \
    do {                                                                                        \
        if ((da)->count + (new_items_count) > (da)->capacity) {                                 \
            if ((da)->capacity == 0) (da)->capacity = DA_DEFAULT_CAP;                           \
            while ((da)->count + (new_items_count) > (da)->capacity) (da)->capacity *= 2;       \
            (da)->items = reallocarray((da)->items, (da)->capacity, sizeof(*(da)->items));      \
            assert((da)->items != NULL && "Buy more RAM");                                      \
        }                                                                                       \
        memcpy((da)->items + (da)->count, (new_items), (new_items_count)*sizeof(*(da)->items)); \
        (da)->count += (new_items_count);                                                       \
    } while (0)

#define da_pop(da, item)                                  \
    do {                                                  \
        assert((da)->count > 0 && "DA ERROR: underflow"); \
        (item) = (da)->items[--(da)->count];              \
    } while (0)

#define da_remove(da, i, item)                                                               \
    do {                                                                                     \
        if ((size_t)(i) == (da)->count-1) {                                                  \
            da_pop((da), (item));                                                            \
        } else {                                                                             \
            assert((i) >= 0 && "Index must be positive.");                                   \
            assert((size_t)(i) < (da)->count && "Index must be less than the array count."); \
            (item) = (da)->items[i];                                                         \
            memmove((da)->items+i, (da)->items+i+1, ((da)->count-i-1)*sizeof(*(da)->items)); \
            (da)->count--;                                                                   \
        }                                                                                    \
    } while (0)

// TODO: piu' efficiente: `item=da->items[0]; da->items++;` ma siamo sicuri? e il count?
#define da_pop_front(da, item) da_remove((da), 0, (item))

#define da_for(da, i) for ((i) = 0; (size_t)(i) < (da).count; (i)++)

#define da_foreach(da, item) for ((item) = (da).items; (item) < (da).items+(da).count; (item)++)

// TODO: lo tengo? non serve davvero
#define da_enumerate(da, i, item) for ((i) = 0, (item) = (da).items[0]; (size_t)(i) < (da).count; (i)++, (item)=(da).items[i])

#define da_clear(da) (da)->count = 0; // NOTE: does not free
#define da_free(da) free((da)->items);

#define da_is_empty(da) ((da)->count == 0)

#endif // INCLUDE_DA_H
