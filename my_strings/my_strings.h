#ifndef INCLUDE_STR_H
#define INCLUDE_STR_H

#include "dynamic_arrays.h"
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
void s_insert(String *s, char c, size_t i);

char s_pop(String *s);
void s_remove(String *s, size_t i);
void s_remove_first(String *s);

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
bool s_eq_cstr(String a, char *b);

int s_cmp_s(String a, String b);
bool s_eq_s(String a, String b);

void s_to_cstr(String s, char **cstr);

String s_clone(String s);

// TODO
void s_copy(String *dest, String src);
void s_cat(String *dest, String src);
int s_char_index(String s, char c);
int s_is_char_in(String s, char c);


// StringView

typedef struct
{
    char *data;
    size_t count;
} StringView;

#define SV_FMT "%.*s"
#define SV_ARG(s) (int) (s).count, (s).data

StringView sv_from_s(String s);
StringView sv_from_parts(const char *data, size_t count);

StringView sv_trim_left(StringView sv);
StringView sv_trim_right(StringView sv);
StringView sv_trim(StringView sv);

StringView sv_chop_by_char(StringView *sv, char c);
char sv_chop_first(StringView *sv);

bool sv_eq_cstr(StringView a, char *b);

#endif // INCLUDE_STR_H

#ifdef MY_STRINGS_IMPLEMENTATION

void s_push(String *s, char c) { da_push(s, c); }
void s_push_null(String *s) { da_push_many(s, "", 1); }

void s_push_str(String *s, char *str, size_t len) { da_push_many(s, str, len); }
void s_push_cstr(String *s, char *str) { da_push_many(s, str, strlen(str)); }
void s_push_fstr(String *s, char *fmt, ...)
{
    if (!s || !fmt) return;

    va_list ap1;
    va_list ap2;
    va_start(ap1, fmt);
    va_copy(ap2, ap1);

    int len = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    if (len < 0) {
        va_end(ap1);
        return;
    }

    char *buffer = malloc(len+1);
    if (!buffer) {
        va_end(ap1);
        return;
    }

    vsnprintf(buffer, len+1, fmt, ap1);
    va_end(ap1);

    s_push_cstr(s, buffer);
    free(buffer);
}
void s_insert(String *s, char c, size_t i) { da_insert(s, c, i); }

char s_pop(String *s) { return da_pop(s); }
void s_remove(String *s, size_t i) { da_remove(s, i); }
void s_remove_first(String *s) { da_remove_first(s); }

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
bool s_eq_cstr(String a, char *b)  { return s_cmp_cstr(a, b) == 0; }

int s_cmp_s(String a, String b) { return s_cmp_cstr(a, b.items); }
bool s_eq_s(String a, String b)  { return a.count == b.count && s_eq_cstr(a, b.items); }

void s_to_cstr(String s, char **cstr) { strncpy(*cstr, s.items, s.count); }

// StringView
StringView sv_from_s(String s) { return (StringView){ .data=s.items, .count=s.count }; }
StringView sv_from_parts(const char *data, size_t count)
{
    StringView sv;
    sv.count = count;
    sv.data = data;
    return sv;
}

StringView sv_trim_left(StringView sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i += 1;
    }

    return sv_from_parts(sv.data + i, sv.count - i);
}

StringView sv_trim_right(StringView sv)
{
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
        i += 1;
    }

    return sv_from_parts(sv.data, sv.count - i);
}

StringView sv_trim(StringView sv)
{
    return sv_trim_right(sv_trim_left(sv));
}

StringView sv_chop_by_char(StringView *sv, char c)
{
    size_t i = 0;
    while (i < sv->count && sv->data[i] != c) {
        i += 1;
    }

    StringView result = sv_from_parts(sv->data, i);

    if (i < sv->count) {
        sv->count -= i + 1;
        sv->data  += i + 1;
    } else {
        sv->count -= i;
        sv->data  += i;
    }

    return result;
}

char sv_chop_first(StringView *sv)
{
    if (sv->count == 0) return 0;

    char c = sv->data[0];

    sv->data++;
    sv->count--;

    return c;
}

bool sv_eq_cstr(StringView a, char *b)  { return a.count == strlen(b) && strncmp(a.data, b, a.count) == 0; }

#endif // MY_STRINGS_IMPLEMENTATION
