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

#endif // WYRM_CORE_H
