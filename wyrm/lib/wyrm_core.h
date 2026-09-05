#ifndef WYRM_CORE_H
#define WYRM_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>

typedef enum {
    VAL_NULL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_ARRAY,
    VAL_STRUCT,
    VAL_RAW_PTR,
    VAL_ERROR
} ValueType;

struct Value;
struct WyrmArena;
typedef struct WyrmArena WyrmArena;

typedef struct {
    struct Value *data;
    int size;
    int capacity;
} ValArray;

typedef struct WyrmStruct {
    char *type_name;
    int field_count;
    char **field_names;
    struct Value *fields;
    int ref_count;
} WyrmStruct;

typedef struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        char *string;
        ValArray *array;
        WyrmStruct *structure;
        void *raw_ptr;
    } as;
} Value;

void wyrm_check_oom(void *ptr, const char *context);

// Core constructors and type conversions
Value val_null();
Value val_bool(bool b);
Value val_number(double n);
Value val_string(const char *s);
Value val_error(const char *s);
Value val_error_val(Value msg);
Value val_array_create(int count);
Value val_array_init(int count, ...);
bool val_to_bool(Value v);
char* val_to_str_ptr(Value v);

// Builtin operations
void val_print(int count, ...);
Value val_input(Value prompt);
Value val_len(Value v);
Value val_type(Value v);
Value val_int(Value v);
Value val_float(Value v);
Value val_str(Value v);
Value val_abs(Value v);
Value val_min(int count, ...);
Value val_max(int count, ...);
Value val_round(Value v);
Value val_pow(Value x, Value y);

// Arithmetic and logical operations
Value val_add(Value a, Value b);
Value val_sub(Value a, Value b);
Value val_mul(Value a, Value b);
Value val_div(Value a, Value b);
Value val_mod(Value a, Value b);
Value val_floordiv(Value a, Value b);
Value val_eq(Value a, Value b);
Value val_ne(Value a, Value b);
Value val_lt(Value a, Value b);
Value val_gt(Value a, Value b);
Value val_le(Value a, Value b);
Value val_ge(Value a, Value b);
Value val_and(Value a, Value b);
Value val_or(Value a, Value b);
Value val_not(Value a);

// Array and Slice operations
Value val_array_get(Value arr, Value index);
Value val_array_set(Value arr, Value index, Value val);
Value val_array_append(Value arr, Value item);
Value val_array_pop(Value arr);
Value val_array_slice(Value arr, Value start, Value end);

// Struct operations
Value val_struct_create(const char *type_name, int field_count, const char **field_names, const Value *initial_fields);
Value val_struct_get(Value s, const char *field_name);
Value val_struct_set(Value s, const char *field_name, Value new_val);

// Raw memory safety operations
Value val_raw_malloc(Value size);
Value val_raw_realloc(Value ptr, Value size);
Value val_raw_free(Value ptr);
Value val_raw_ptr(void *p);
Value val_arena_alloc(WyrmArena *a, Value size);
Value val_arena_reset(WyrmArena *a);

extern Value wyrm_sys_args;
void val_init_sys_args(int argc, char *argv[]);
Value val_sys_args();

Value val_read_file(Value path);
Value val_write_file(Value path, Value content);
Value val_exit(Value code);
Value val_system(Value cmd);
Value val_getenv(Value name);

// -------------------------------------------------------------------------
// Value Lifetime Management (Drop / Copy)
// -------------------------------------------------------------------------
// val_drop: destructor for a Value. Recursively frees all heap-allocated
//   resources owned by v (strings, array data, nested elements).
//   Call this when a Value is no longer needed and the caller owns it.
//   No-op for value types (null, bool, number, raw_ptr).
void val_drop(Value v);

// val_copy: deep copy a Value, producing a new independently owned Value.
//   The caller is responsible for calling val_drop on the returned Value
//   when it is no longer needed.
//   No-op (returns v) for value types that have no heap allocation.
Value val_copy(Value v);

// LLVM IR Wrapper functions
void llvm_val_null(Value *res);
void llvm_val_bool(Value *res, bool b);
void llvm_val_number(Value *res, double n);
void llvm_val_string(Value *res, const char *s);
void llvm_val_error(Value *res, const char *s);
void llvm_val_array_create(Value *res, int count);
void llvm_val_array_init(Value *res, int count, ...);
bool llvm_val_to_bool(Value *v);
void llvm_val_print(int count, ...);
void llvm_val_input(Value *res, Value *prompt);
void llvm_val_len(Value *res, Value *v);
void llvm_val_type(Value *res, Value *v);
void llvm_val_int(Value *res, Value *v);
void llvm_val_float(Value *res, Value *v);
void llvm_val_str(Value *res, Value *v);
void llvm_val_abs(Value *res, Value *v);
void llvm_val_round(Value *res, Value *v);
void llvm_val_pow(Value *res, Value *x, Value *y);
void llvm_val_add(Value *res, Value *a, Value *b);
void llvm_val_sub(Value *res, Value *a, Value *b);
void llvm_val_mul(Value *res, Value *a, Value *b);
void llvm_val_div(Value *res, Value *a, Value *b);
void llvm_val_mod(Value *res, Value *a, Value *b);
void llvm_val_floordiv(Value *res, Value *a, Value *b);
void llvm_val_eq(Value *res, Value *a, Value *b);
void llvm_val_ne(Value *res, Value *a, Value *b);
void llvm_val_lt(Value *res, Value *a, Value *b);
void llvm_val_gt(Value *res, Value *a, Value *b);
void llvm_val_le(Value *res, Value *a, Value *b);
void llvm_val_ge(Value *res, Value *a, Value *b);
void llvm_val_and(Value *res, Value *a, Value *b);
void llvm_val_or(Value *res, Value *a, Value *b);
void llvm_val_not(Value *res, Value *a);
void llvm_val_array_get(Value *res, Value *arr, Value *index);
void llvm_val_array_set(Value *arr, Value *index, Value *val);
void llvm_val_array_append(Value *res, Value *arr, Value *item);
void llvm_val_array_pop(Value *res, Value *arr);
void llvm_val_array_slice(Value *res, Value *arr, Value *start, Value *end);
void llvm_val_raw_malloc(Value *res, Value *size);
void llvm_val_raw_realloc(Value *res, Value *ptr, Value *size);
void llvm_val_raw_free(Value *res, Value *ptr);
void llvm_val_raw_ptr(Value *res, void *p);
void llvm_val_arena_alloc(Value *res, WyrmArena *a, Value *size);
void llvm_val_arena_reset(Value *res, WyrmArena *a);
void llvm_val_read_file(Value *res, Value *path);
void llvm_val_error_val(Value *res, Value *msg);
void llvm_val_write_file(Value *res, Value *path, Value *content);
void llvm_val_exit(Value *res, Value *code);
void llvm_val_system(Value *res, Value *cmd);
void llvm_val_getenv(Value *res, Value *name);

// String ops wrappers
void llvm_val_split(Value *res, Value *a, Value *b);
void llvm_val_join(Value *res, Value *a, Value *b);
void llvm_val_trim(Value *res, Value *a);
void llvm_val_upper(Value *res, Value *a);
void llvm_val_lower(Value *res, Value *a);
void llvm_val_contains(Value *res, Value *a, Value *b);
void llvm_val_replace(Value *res, Value *a, Value *b, Value *c);
void llvm_val_starts_with(Value *res, Value *a, Value *b);
void llvm_val_ends_with(Value *res, Value *a, Value *b);
void llvm_val_char_at(Value *res, Value *a, Value *b);
void llvm_val_ord_val(Value *res, Value *a);
void llvm_val_chr_val(Value *res, Value *a);
void llvm_val_to_bytes(Value *res, Value *a);
void llvm_val_from_bytes(Value *res, Value *a);
void llvm_val_copy(Value *dest, Value *src);
WyrmArena* val_arena_create_wrapper(Value *size);
void llvm_val_struct_create(Value *res, const char *type_name, int field_count);
void llvm_val_struct_get(Value *res, Value *s, const char *field_name);
void llvm_val_struct_set(Value *s, const char *field_name, Value *val);
void llvm_val_from_i64(Value *res, int64_t v);
void llvm_val_from_u8(Value *res, uint8_t v);
void llvm_val_from_f32(Value *res, float v);
void llvm_val_from_bool(Value *res, bool v);
void llvm_val_drop(Value *v);

// Standard library wrappers
void llvm_val_json_parse(Value *res, Value *s);
void llvm_val_json_encode(Value *res, Value *v);
void llvm_val_json_pretty(Value *res, Value *v, Value *indent);
void llvm_val_json_get(Value *res, Value *obj, Value *key);
void llvm_val_json_has(Value *res, Value *obj, Value *key);
void llvm_val_json_set(Value *res, Value *obj, Value *key, Value *val);
void llvm_val_json_object(Value *res);

void llvm_val_yaml_parse(Value *res, Value *s);
void llvm_val_yaml_encode(Value *res, Value *v);

void llvm_val_map_new(Value *res);
void llvm_val_map_set(Value *res, Value *m, Value *k, Value *v);
void llvm_val_map_get(Value *res, Value *m, Value *k);
void llvm_val_map_has(Value *res, Value *m, Value *k);
void llvm_val_map_del(Value *res, Value *m, Value *k);
void llvm_val_map_keys(Value *res, Value *m);
void llvm_val_map_values(Value *res, Value *m);
void llvm_val_map_len(Value *res, Value *m);

void llvm_val_set_new(Value *res);
void llvm_val_set_add(Value *res, Value *s, Value *v);
void llvm_val_set_has(Value *res, Value *s, Value *v);
void llvm_val_set_del(Value *res, Value *s, Value *v);
void llvm_val_set_union(Value *res, Value *a, Value *b);
void llvm_val_set_intersect(Value *res, Value *a, Value *b);
void llvm_val_set_to_array(Value *res, Value *s);

void llvm_val_sdl_init(Value *res);
void llvm_val_sdl_quit(Value *res);
void llvm_val_sdl_window(Value *res, Value *title, Value *w, Value *h);
void llvm_val_sdl_destroy_window(Value *res, Value *win);
void llvm_val_sdl_poll_event(Value *res);
void llvm_val_sdl_clear(Value *res, Value *win, Value *r, Value *g, Value *b);
void llvm_val_sdl_present(Value *res, Value *win);
void llvm_val_sdl_draw_rect(Value *res, Value *win, Value *x, Value *y, Value *w, Value *h, Value *r, Value *g, Value *b);
void llvm_val_sdl_draw_line(Value *res, Value *win, Value *x1, Value *y1, Value *x2, Value *y2, Value *r, Value *g, Value *b);
void llvm_val_sdl_delay(Value *res, Value *ms);
void llvm_val_sdl_ticks(Value *res);

void llvm_val_ffi_open(Value *res, Value *path);
void llvm_val_ffi_sym(Value *res, Value *lib, Value *sym);
void llvm_val_ffi_call(Value *res, Value *fn_ptr, Value *args);
void llvm_val_ffi_close(Value *res, Value *lib);

#endif // WYRM_CORE_H
