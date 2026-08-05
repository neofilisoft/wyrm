#ifndef WYRM_CORE_H
#define WYRM_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>

typedef enum {
    VAL_NULL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_ARRAY,
    VAL_RAW_PTR
} ValueType;

struct Value;
struct WyrmArena;
typedef struct WyrmArena WyrmArena;

typedef struct {
    struct Value *data;
    int size;
    int capacity;
} ValArray;

typedef struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        char *string;
        ValArray *array;
        void *raw_ptr;
    } as;
} Value;

// Core constructors and type conversions
Value val_null();
Value val_bool(bool b);
Value val_number(double n);
Value val_string(const char *s);
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

// Raw memory safety operations
Value val_raw_malloc(Value size);
Value val_raw_realloc(Value ptr, Value size);
Value val_raw_free(Value ptr);
Value val_raw_ptr(void *p);

extern Value wyrm_sys_args;
void val_init_sys_args(int argc, char *argv[]);
Value val_sys_args();

Value val_read_file(Value path);
Value val_write_file(Value path, Value content);
Value val_exit(Value code);
Value val_system(Value cmd);
Value val_getenv(Value name);

// LLVM IR Wrapper functions
void llvm_val_null(Value *res);
void llvm_val_bool(Value *res, bool b);
void llvm_val_number(Value *res, double n);
void llvm_val_string(Value *res, const char *s);
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
void llvm_val_arena_reset(Value *res, WyrmArena *a);
void llvm_val_read_file(Value *res, Value *path);
void llvm_val_write_file(Value *res, Value *path, Value *content);
void llvm_val_exit(Value *res, Value *code);
void llvm_val_system(Value *res, Value *cmd);

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

#endif // WYRM_CORE_H
