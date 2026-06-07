#ifndef CDC_H
#define CDC_H

#include <stdio.h>
#include <ctype.h>
#include "../dynamic_arrays/dynamic_arrays.h"

typedef enum
{
    CDCBUILTINTYPE_INT,
    CDCBUILTINTYPE_FLOAT,
    __cdc_builtin_types_count,
    CDCBUILTINTYPE_UNKNOWN,
} CDCBuiltinType;

static_assert(__cdc_builtin_types_count == 2, "string for each cdc builtin type");
static const char *cdc_builtin_types_strings[] = {
    [CDCBUILTINTYPE_INT]   = "int",
    [CDCBUILTINTYPE_FLOAT] = "float",
};

typedef enum
{
    CDCKIND_BUILTIN,
    CDCKIND_STRUCT,
    __cdc_kinds_count,
    CDCKIND_UNKNOWN,
} CDCKind;

typedef struct CDCStruct CDCStruct;

typedef struct
{
    CDCKind kind;
    union {
        CDCStruct *strct;
        CDCBuiltinType builtin_type;
    };
} CDCType;

typedef struct
{
    CDCType type;
    const char *name;
} CDCField;

struct CDCStruct
{
    const char *name;

    CDCField *items;
    size_t count;
    size_t capacity;
};

typedef struct
{
    CDCStruct *items;
    size_t count;
    size_t capacity;
} CDCStructs;

static CDCStruct cdc_current_struct;
static CDCStructs cdc_structs = {0};
static char *cdc_output_h_filepath = "./__cdc_generated.h";
static char *cdc_output_c_filepath = "./__cdc_generated.c";

extern bool cdc_result;

void cdc_set_output_h_filepath(char *filepath);
void cdc_set_output_c_filepath(char *filepath);

void cdc_define_struct_begin(void);
void cdc_define_struct_end(char *struct_name);

bool cdc_add_field(const char *type, const char *name);
bool cdc_add_field_array(const char *type, const char *name, const size_t len);

static void cdc_generate_struct(const CDCStruct *strct, FILE *outfile);
static void cdc_generate_serialization_declarations(const CDCStruct *strct, FILE *outfile);
static void cdc_generate_deserialization_definitions(const CDCStruct *strct, FILE *outfile);
static void cdc_generate_serialization_declarations(const CDCStruct *strct, FILE *outfile);
static void cdc_generate_deserialization_definitions(const CDCStruct *strct, FILE *outfile);

bool cdc_generate(void);
static bool cdc_generate_h_file(void);
static bool cdc_generate_c_file(void);

#define CDC_INFO "CDC INFO: "
#define CDC_WARNING "CDC WARNING: "
#define CDC_NOTE "CDC NOTE: "
#define CDC_ERROR "CDC ERROR: "
#define CDC_UNREACHABLE "CDC UNREACHABLE: "

#endif // CDC_H

#ifdef CDC_IMPLEMENTATION

bool cdc_result = true;

static CDCBuiltinType cdc_builtin_type_from_string(const char *type)
{
    for (CDCBuiltinType builtin = 0; builtin < __cdc_builtin_types_count; builtin++) {
        if (strcmp(type, cdc_builtin_types_strings[builtin]) == 0) return builtin;
    }
    return CDCBUILTINTYPE_UNKNOWN;
}

static CDCStruct *cdc_struct_from_name(const char *struct_name)
{
    for (size_t i = 0; i < cdc_structs.count; i++) {
        if (strcmp(struct_name, cdc_structs.items[i].name) == 0) return &cdc_structs.items[i];
    }
    return NULL;
}

void cdc_set_output_h_filepath(char *filepath)
{
    cdc_output_h_filepath = strdup(filepath);
}

void cdc_set_output_c_filepath(char *filepath)
{
    cdc_output_c_filepath = strdup(filepath);
}

void cdc_define_struct_begin(void)
{
    cdc_current_struct = (CDCStruct){0};
}

void cdc_define_struct_end(char *struct_name)
{
    cdc_current_struct.name = strdup(struct_name);
    da_push(&cdc_structs, cdc_current_struct);
}

static const char *cdc_type_as_string(CDCType type)
{
    switch (type.kind)
    {
    case CDCKIND_BUILTIN: return cdc_builtin_types_strings[type.builtin_type];
    case CDCKIND_STRUCT: return type.strct->name;
    case CDCKIND_UNKNOWN: return "unknown";

    case __cdc_kinds_count:
    default:
        printf(CDC_UNREACHABLE"cdc kind %u in cdc_type_as_string\n", type.kind);
        exit(1);
    }
}

static CDCType cdc_type_from_string(const char *type)
{
    CDCBuiltinType builtin = cdc_builtin_type_from_string(type);
    if (builtin != CDCBUILTINTYPE_UNKNOWN) {
        return (CDCType){
            .kind = CDCKIND_BUILTIN,
            .builtin_type = builtin
        };
    }

    CDCStruct *strct = cdc_struct_from_name(type);
    if (strct) {
        return (CDCType){
            .kind = CDCKIND_STRUCT,
            .strct = strct
        };
    }

    return (CDCType){ .kind = CDCKIND_UNKNOWN };
}

bool cdc_add_field(const char *type, const char *name)
{
}

bool cdc_add_field_array(const char *type, const char *name, const size_t len)
{
    if (len == 0) {
        printf(CDC_ERROR"Array `%s` should not have length 0\n", name);
        cdc_result = false;
        goto ret;
    }

    CDCType cdctype = cdc_type_from_string(type);
    if (cdctype.kind == CDCKIND_UNKNOWN) {
        printf(CDC_ERROR"Unknown type `%s`\n", type);
        if (strchr(type, '*')) {
            printf(CDC_NOTE"Pointers serialization/deserialization is not available\n");
        }
        cdc_result = false;
        goto ret;
    }

    CDCType name_cdctype = cdc_type_from_string(name);
    if (name_cdctype.kind != CDCKIND_UNKNOWN) {
        printf(CDC_ERROR"Field name cannot have name of type (%s)\n", name);
        cdc_result = false;
        goto ret;
    }

    da_foreach (cdc_current_struct, field) {
        if (strcmp(name, field->name) == 0) {
            printf(CDC_ERROR"Redefinition of field `%s` (types are `%s` and `%s`)\n", name, cdc_type_as_string(cdctype),
                    cdc_type_as_string(field->type));
            cdc_result = false;
            goto ret;
        }
    }

    CDCField field = {
        .type = cdctype,
        .name = strdup(name),
        .len  = len
    };
    da_push(&cdc_current_struct, field);

    cdc_result &= true;
ret:
    return cdc_result;
}

static void cdc_generate_struct(const CDCStruct *strct, FILE *outfile)
{
    fprintf(outfile, "typedef struct\n");
    fprintf(outfile, "{\n");
    da_foreach (*strct, field) {
        fprintf(outfile, "    %s %s;\n", cdc_type_as_string(field->type), field->name);
    }
    fprintf(outfile, "} %s;\n", strct->name);
}

static void cdc_generate_serialization_declarations(const CDCStruct *strct, FILE *outfile)
{
    fprintf(outfile, "bool serialize_struct_%s(const %s *s, FILE *f);\n", strct->name, strct->name);
}

static void cdc_generate_serialization_definitions(const CDCStruct *strct, FILE *outfile)
{
    fprintf(outfile, "bool serialize_struct_%s(const %s *s, FILE *f)\n", strct->name, strct->name);
    fprintf(outfile, "{\n");

    da_foreach (*strct, field) {
        switch (field->type.kind)
        {
        case CDCKIND_BUILTIN:
            if (field->len == 1) {
                fprintf(outfile, "    if (fwrite(&s->%s, sizeof(%s), 1, f) != 1) return false;\n", field->name,
                        cdc_type_as_string(field->type));
            } else {
                fprintf(outfile, "    if (fwrite(&s->%s, sizeof(size_t), 1, f) != 1) return false;\n", field->name,
                fprintf(outfile, "    if (fwrite(&s->%s, sizeof(%s), %zu, f) != %zu) return false;\n", field->name,
                        cdc_type_as_string(field->type), field->len, field->len);
                fprintf(outfile, "    if (fwrite(&s->%s, sizeof(%s), %zu, f) != %zu) return false;\n", field->name,
                        cdc_type_as_string(field->type), field->len, field->len);
            }
            break;

        case CDCKIND_STRUCT:
            fprintf(outfile, "    if (!serialize_struct_%s(&s->%s, f)) return false;\n", field->type.strct->name,
                    field->name);
            break;

        case CDCKIND_UNKNOWN:
        case __cdc_kinds_count:
        default:
            printf(CDC_UNREACHABLE"cdc kind %u in cdc_generate_serialization_definitions\n", field->type.kind);
            exit(1);
        }
    }
    fprintf(outfile, "    return true;\n");
    fprintf(outfile, "}\n");
}

static void cdc_generate_deserialization_declarations(const CDCStruct *strct, FILE *outfile)
{
    fprintf(outfile, "bool deserialize_struct_%s(%s *s, FILE *f);\n", strct->name, strct->name);
}

static void cdc_generate_deserialization_definitions(const CDCStruct *strct, FILE *outfile)
{
    fprintf(outfile, "bool deserialize_struct_%s(%s *s, FILE *f)\n", strct->name, strct->name);
    fprintf(outfile, "{\n");

    da_foreach (*strct, field) {
        switch (field->type.kind)
        {
        case CDCKIND_BUILTIN:
            fprintf(outfile, "    if (fread(&s->%s, sizeof(%s), 1, f) != 1) return false;\n", field->name,
                    cdc_type_as_string(field->type));
            break;

        case CDCKIND_STRUCT:
            fprintf(outfile, "    if (!deserialize_struct_%s(&s->%s, f)) return false;\n", field->type.strct->name,
                    field->name);
            break;

        case CDCKIND_UNKNOWN:
        case __cdc_kinds_count:
        default:
            printf(CDC_UNREACHABLE"cdc kind %u in cdc_generate_deserialization_definitions\n", field->type.kind);
            exit(1);
        }
    }
    fprintf(outfile, "    return true;\n");
    fprintf(outfile, "}\n");
}

static bool cdc_generate_h_file(void)
{
    FILE *outfile = fopen(cdc_output_h_filepath, "w");
    if (!outfile) {
        printf(CDC_ERROR"Could not open cdc output header file `%s`\n", cdc_output_h_filepath);
        cdc_result = false;
        goto ret;
    }

    printf(CDC_INFO"Generating cdc output header file `%s`\n", cdc_output_h_filepath);

    fprintf(outfile, "// This file has been generated by cdc, do not modify.\n\n");

    char header_guard_name[128]; 
    char *h_filename = strrchr(cdc_output_h_filepath, '/');
    h_filename = h_filename ? h_filename + 1 : cdc_output_h_filepath;
    snprintf(header_guard_name, sizeof(header_guard_name), "_CDC_%s", h_filename);
    for (size_t i = 0; i < strlen(header_guard_name); i++) {
        char c = header_guard_name[i];
        if (!isalpha(c)) continue;
        header_guard_name[i] = toupper(c);
    }
    char *dot = strrchr(header_guard_name, '.');
    if (dot) *dot = '_';
    fprintf(outfile, "#ifndef %s\n", header_guard_name);
    fprintf(outfile, "#define %s\n\n", header_guard_name);

    if (!da_is_empty(&cdc_structs)) {
        da_foreach (cdc_structs, strct) {
            cdc_generate_struct(strct, outfile);
            cdc_generate_serialization_declarations(strct, outfile);
            cdc_generate_deserialization_declarations(strct, outfile);
            fprintf(outfile, "\n");
        }
    }

    fprintf(outfile, "#endif // %s\n", header_guard_name);

    fclose(outfile);

    cdc_result &= true;
ret:
    return cdc_result; 
}

static bool cdc_generate_c_file(void)
{
    FILE *outfile = fopen(cdc_output_c_filepath, "w");
    if (!outfile) {
        printf(CDC_ERROR"Could not open cdc output c file `%s`\n", cdc_output_c_filepath);
        cdc_result = false;
        goto ret;
    }

    printf(CDC_INFO"Generating cdc output c file `%s`\n", cdc_output_c_filepath);

    fprintf(outfile, "// This file has been generated by cdc, do not modify.\n\n");
    fprintf(outfile, "#include <stdbool.h>\n");
    fprintf(outfile, "#include \"%s\"\n\n", cdc_output_h_filepath);

    if (!da_is_empty(&cdc_structs)) {
        da_foreach (cdc_structs, strct) {
            cdc_generate_serialization_definitions(strct, outfile);
            cdc_generate_deserialization_definitions(strct, outfile);
            fprintf(outfile, "\n");
        }
    }

    fclose(outfile);

    cdc_result &= true;
ret:
    return cdc_result; 
}

bool cdc_generate(void)
{
    if (!cdc_generate_h_file()) {
        cdc_result = false;
        goto ret;
    }
    if (!cdc_generate_c_file()) {
        cdc_result = false;
        goto ret;
    }

    cdc_result &= true;
ret:
    return cdc_result;
}

// TODO devo tenerlo?
//#define DA_IMPLEMENTATION
//#include "../dynamic_arrays/dynamic_arrays.h"


#endif // CDC_IMPLEMENTATION
