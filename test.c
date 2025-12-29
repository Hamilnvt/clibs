#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#include "dynamic_arrays.h"

#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_RESET   "\x1b[0m"

typedef struct
{
    int *items;
    size_t count;
    size_t capacity;
} IntArray;

typedef bool (* test_func) (void);
typedef struct
{
    const char *name;
    test_func func;
} Test;

static char errstr[4096];
void set_errstr(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    memset(errstr, 0, sizeof(errstr));
    vsnprintf(errstr, sizeof(errstr), fmt, ap);
    va_end(ap);
}

#define BIG_NUMBER 10000

#define TEST_ASSERT(cond, fmt, ...)         \
    do {                                    \
        if (!(cond)) {                      \
            set_errstr(fmt, ##__VA_ARGS__); \
            result = false;                 \
            goto defer;                     \
        }                                   \
    } while (0)

bool test_push(void)
{
    bool result = true;
    IntArray A = {0};
    for (int i = 0; i < BIG_NUMBER; i++) {
        da_push(&A, i);
    }

    TEST_ASSERT(A.count == BIG_NUMBER, "count != %d, got %zu", BIG_NUMBER, A.count);

    for (int i = 0; i < BIG_NUMBER; i++) {
        TEST_ASSERT(A.items[i] == i, "A.items[%d] != %d, got %d", i, i, A.items[i]);
    }

defer:
    da_free(&A);
    return result;
}

bool test_for(void)
{
    bool result = true;
    IntArray A = {0};
    for (int i = 0; i < BIG_NUMBER; i++) {
        da_push(&A, i);
    }

    int c = 0;
    da_for(A, it) {
        A.items[it] *= 2;
        c++;
    }

    TEST_ASSERT(c == BIG_NUMBER, "for count != %d, got %d", BIG_NUMBER, c);

    for (int i = 0; i < BIG_NUMBER; i++) {
        TEST_ASSERT(A.items[i] == 2*i, "A.items[%d] != %d, got %d", i, 2*i, A.items[i]);
    }
    
defer:
    da_free(&A);
    return result;
}

bool test_insert_and_first(void)
{
    bool result = true;
    IntArray A = {0};

    // State: []
    da_push(&A, 10);
    da_push(&A, 20);
    // State: [10, 20]
 
    // Insert at index 1
    da_insert(&A, 99, 1);
    // Expected: [10, 99, 20]

    TEST_ASSERT(A.count == 3, "Count should be 3, got %zu", A.count);
    TEST_ASSERT(A.items[0] == 10, "Idx 0 should be 10, got %d", A.items[0]);
    TEST_ASSERT(A.items[1] == 99, "Idx 1 should be 99, got %d", A.items[1]);
    TEST_ASSERT(A.items[2] == 20, "Idx 2 should be 20, got %d", A.items[2]);

    // Push First
    da_push_first(&A, 5);
    // Expected: [5, 10, 99, 20]
 
    TEST_ASSERT(A.count == 4, "Count should be 4 after push_first, got %zu", A.count);
    TEST_ASSERT(A.items[0] == 5, "Idx 0 should be 5, got %d", A.items[0]);
    TEST_ASSERT(A.items[2] == 99, "Idx 2 should be 99, got %d", A.items[2]);

    da_clear(&A);
    for (int i = 0; i < BIG_NUMBER; i++) {
        size_t index = rand()%(A.count+1);
        da_insert(&A, i, index);
    }

    TEST_ASSERT(A.count == BIG_NUMBER, "Count should be BIG_NUMBER after da_insert loop, got %zu", A.count);
    
defer:
    da_free(&A);
    return result;
}

bool test_push_many(void) {
    bool result = true;
    IntArray A = {0};
    
    da_push(&A, 1);
    da_push(&A, 2);

    int many[] = {3, 4, 5, 6, 7};
    da_push_many(&A, many, 5);
    // Expected: [1, 2, 3, 4, 5, 6, 7]

    TEST_ASSERT(A.count == 7, "Count should be 7, got %zu", A.count);
    for (int i = 0; i < 7; ++i) {
        TEST_ASSERT(A.items[i] == i + 1, "Item %d incorrect", i);
    }

    da_clear(&A);
    int *many_many = malloc(sizeof(int)*BIG_NUMBER);
    for (int i = 0; i < BIG_NUMBER; i++) {
        many_many[i] = i; 
    }
    da_push_many(&A, many_many, BIG_NUMBER);
    TEST_ASSERT(A.count == BIG_NUMBER, "Count should be BIG_NUMBER, got %zu", A.count);

defer:
    da_free(&A);
    if (many_many) free(many_many);
    return result;
}

bool test_remove_and_pop(void)
{
    bool result = true;
    IntArray A = {0};
    // Setup: [0, 1, 2, 3, 4]
    for (int i = 0; i < 5; i++) da_push(&A, i);

    // Test da_pop (removes from end) -> Removes 4
    int popped = da_pop(&A); 
    TEST_ASSERT(popped == 4, "Pop expected 4, got %d", popped);
    TEST_ASSERT(A.count == 4, "Count should be 4");

    // Test da_remove_first (removes from start) -> Removes 0
    popped = da_get(A, 0);
    da_remove_first(&A);
    TEST_ASSERT(popped == 0, "Remove First expected 0, got %d", popped);
    TEST_ASSERT(A.count == 3, "Count should be 3");
    TEST_ASSERT(A.items[0] == 1, "First item should now be 1");

    // Test da_remove (remove index 1) -> Array is currently [1, 2, 3]. Remove '2'.
    popped = da_get(A, 1);
    da_remove(&A, 1);
    TEST_ASSERT(popped == 2, "Remove index 1 expected 2, got %d", popped);
    TEST_ASSERT(A.count == 2, "Count should be 2");
    
    // Remaining should be [1, 3]
    TEST_ASSERT(A.items[0] == 1, "Index 0 is 1");
    TEST_ASSERT(A.items[1] == 3, "Index 1 is 3");

defer:
    da_free(&A);
    return result;
}

bool test_clear(void)
{
    bool result = true;
    IntArray A = {0};
    da_push(&A, 1);
    
    da_clear(&A);
    TEST_ASSERT(A.count == 0, "Count should be 0 after clear");
    // Capacity should remain (memory not freed)
    TEST_ASSERT(A.capacity > 0, "Capacity should remain > 0");
    TEST_ASSERT(da_is_empty(&A), "da_is_empty should return true");

defer:
    da_free(&A);
    return result;
}

Test tests[] = {
    {"push", test_push},
    {"insert & push_first", test_insert_and_first},
    {"push_many", test_push_many},
    {"remove & pop", test_remove_and_pop},
    {"clear", test_clear},
    {"for", test_for},
};
const size_t total = sizeof(tests)/sizeof(tests[0]);

int main(void)
{
    srand(time(NULL));

    printf("Start testing\n\n");

    size_t passed = 0;
    for (size_t i = 0; i < total; i++) {
        printf("Test %zu: %-20s ... ", i + 1, tests[i].name);
        fflush(stdout);
        bool result = tests[i].func(); 
        if (result) {
            passed++;
            printf(COLOR_GREEN "OK" COLOR_RESET "\n");
        } else {
            printf(COLOR_RED "FAIL" COLOR_RESET "\n");
            printf("%s\n", errstr);
        }
        printf("\n");
    }

    printf("\nResults:\n");
    printf("  - passed: %zu/%zu\n", passed, total);

    return (passed == total) ? 0 : 1;
}
