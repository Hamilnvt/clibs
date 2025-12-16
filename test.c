#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

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
    const int len = 10000;
    for (int i = 0; i < len; i++) {
        da_push(&A, i);
    }

    TEST_ASSERT(A.count == (size_t)len, "count != %d, is %zu", len, A.count);

    for (int i = 0; i < len; i++) {
        TEST_ASSERT(A.items[i] == i, "A.items[%d] != %d, is %d", i, i, A.items[i]);
    }

defer:
    da_free(&A);
    return result;
}

bool test_for(void)
{
    bool result = true;
    IntArray A = {0};
    const int len = 10000;
    for (int i = 0; i < len; i++) {
        da_push(&A, i);
    }

    int c = 0;
    da_for(A, it) {
        A.items[it] *= 2;
        c++;
    }

    TEST_ASSERT(c == len, "for count != %d, is %d", len, c);

    for (int i = 0; i < len; i++) {
        TEST_ASSERT(A.items[i] == 2*i, "A.items[%d] != %d, is %d", i, 2*i, A.items[i]);
    }
    
defer:
    da_free(&A);
    return result;
}

Test tests[] = {
    {"push", test_push},

    {"for", test_for},
};
const size_t total = sizeof(tests)/sizeof(tests[0]);

int main(void)
{
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
