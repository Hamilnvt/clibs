#ifndef INCLUDE_STR_H
#define INCLUDE_STR_H

#include "./dynamic_arrays.h"
#include <stdarg.h>

typedef struct
{
    char *items;
    size_t count;
    size_t capacity;
} String;

void s_push(String *s, char c);
void s_push_null(String *s);
void s_push_str(String *s, char *str, size_t len);
void s_push_cstr(String *s, char *str);
void s_push_fstr(String *s, char *fmt, ...);
String s_from_c(char c);
String s_from_str(char *str, size_t len);
String s_from_cstr(char *str);
String s_from_s(String other);

#define S_FMT "%.*s"
#define S_ARG(s) (int) (s).count, (s).items
void s_print(String s);
size_t s_len(String s);
bool s_is_empty(String s);

void s_clear(String *s);
void s_free(String *s);

int s_cmp_cstr(String a, char *b);
int s_eq_cstr(String a, char *b);

int s_cmp_s(String a, String b);
int s_eq_s(String a, String b);

void s_to_cstr(String s, char **cstr);

String s_clone(String s);

// TODO
void s_copy(String *dest, String src);
void s_cat(String *dest, String src);
int s_char_index(String s, char c);
int s_is_char_in(String s, char c);

#endif // INCLUDE_STR_H

#ifdef STRINGS_IMPLEMENTATION

void s_push(String *s, char c) { da_push(s, c); }
void s_push_null(String *s) { da_push_many(s, "", 1); }

void s_push_str(String *s, char *str, size_t len) { da_push_many(s, str, len); }
void s_push_cstr(String *s, char *str) { da_push_many(s, str, strlen(str)); }
void s_push_fstr(String *s, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    s_push_cstr(s, buf);
}

//String s_from_c(char c)
//{
//    String s = {0};
//    s_push(&s, c);
//    return s;
//}

//String s_from_str(char *str, size_t len)
//{
//    String s = {0};
//    s_push_str(&s, str, len);
//    return s;
//}

//String s_from_cstr(char *str)
//{
//    String s = {0};
//    s_push_cstr(&s, str);
//    return s;
//}

String s_clone(String other)
{
    String s = {0};
    s_push_str(&s, other.items, other.count);
    return s;
}

void s_print(String s) { printf(S_FMT, S_ARG(s)); }

size_t s_len(String s) { return s.count; } // TODO: non necessariamente vero se all'interno di items c'e' un '\0'

bool s_is_empty(String s) { return s.count == 0; }

void s_clear(String *s) { da_clear(s); }
void s_free(String *s) { da_free(s); }

int s_cmp_cstr(String a, char *b) { return strncmp(a.items, b, a.count); }
int s_eq_cstr(String a, char *b)  { return s_cmp_cstr(a, b) == 0; }

int s_cmp_s(String a, String b) { return s_cmp_cstr(a, b.items); }
int s_eq_s(String a, String b)  { return s_eq_cstr(a, b.items); }

void s_to_cstr(String s, char **cstr) { strncpy(*cstr, s.items, s.count); }

#endif // STRINGS_IMPLEMENTATION
