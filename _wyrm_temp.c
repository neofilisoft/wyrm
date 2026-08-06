
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


#include <stddef.h>

// Arena Allocator structure
typedef struct WyrmArena {
    char   *buf;        // raw memory block
    size_t  cap;        // total capacity in bytes
    size_t  used;       // bytes allocated so far
    int     freed;      // 1 after arena_reset(), prevents alloc after free
} WyrmArena;

// Arena operations
WyrmArena *arena_create(size_t cap);
void *arena_alloc(WyrmArena *a, size_t sz);
void arena_reset(WyrmArena *a);
void arena_destroy(WyrmArena *a);



// String & Data helper declarations
Value val_split(Value s, Value sep);
Value val_join(Value sep, Value lst);
Value val_trim(Value s);
Value val_upper(Value s);
Value val_lower(Value s);
Value val_contains(Value s, Value sub);
Value val_replace(Value s, Value old, Value sub_new);
Value val_starts_with(Value s, Value prefix);
Value val_ends_with(Value s, Value suffix);
Value val_char_at(Value s, Value idx);
Value val_ord_val(Value c);
Value val_chr_val(Value n);
Value val_to_bytes(Value s);
Value val_from_bytes(Value lst);


Value val_null() {
    Value v;
    v.type = VAL_NULL;
    return v;
}

Value val_bool(bool b) {
    Value v;
    v.type = VAL_BOOL;
    v.as.boolean = b;
    return v;
}

Value val_number(double n) {
    Value v;
    v.type = VAL_NUMBER;
    v.as.number = n;
    return v;
}

Value val_string(const char *s) {
    Value v;
    v.type = VAL_STRING;
    v.as.string = strdup(s ? s : "");
    return v;
}

Value val_array_create(int count) {
    Value v;
    v.type = VAL_ARRAY;
    v.as.array = (ValArray*)malloc(sizeof(ValArray));
    if (!v.as.array) { fprintf(stderr, "Out of memory\n"); exit(1); }
    v.as.array->size = count;
    v.as.array->capacity = count > 0 ? count : 0;
    if (count > 0) {
        v.as.array->data = (Value*)malloc((size_t)count * sizeof(Value));
        if (!v.as.array->data) { fprintf(stderr, "Out of memory\n"); exit(1); }
    } else {
        v.as.array->data = NULL;
    }
    return v;
}

Value val_array_init(int count, ...) {
    Value v = val_array_create(count);
    if (count > 0) {
        va_list args;
        va_start(args, count);
        for (int i = 0; i < count; i++) {
            v.as.array->data[i] = va_arg(args, Value);
        }
        va_end(args);
    }
    return v;
}

bool val_to_bool(Value v) {
    switch (v.type) {
        case VAL_NULL: return false;
        case VAL_BOOL: return v.as.boolean;
        case VAL_NUMBER: return v.as.number != 0.0;
        case VAL_STRING: return strlen(v.as.string) > 0;
        case VAL_ARRAY: return v.as.array->size > 0;
        case VAL_RAW_PTR: return v.as.raw_ptr != NULL;
    }
    return false;
}

char* val_to_str_ptr(Value v) {
    char buf[128];
    switch (v.type) {
        case VAL_NULL: return strdup("null");
        case VAL_BOOL: return strdup(v.as.boolean ? "true" : "false");
        case VAL_RAW_PTR: {
            char ptrbuf[64];
            snprintf(ptrbuf, sizeof(ptrbuf), "<raw_ptr %p>", v.as.raw_ptr);
            return strdup(ptrbuf);
        }
        case VAL_NUMBER:
            if (v.as.number == floor(v.as.number)) {
                snprintf(buf, sizeof(buf), "%.0f", v.as.number);
            } else {
                snprintf(buf, sizeof(buf), "%g", v.as.number);
            }
            return strdup(buf);
        case VAL_STRING: return strdup(v.as.string);
        case VAL_ARRAY: {
            int cap = 256;
            char *res = malloc(cap);
            strcpy(res, "[");
            for (int i = 0; i < v.as.array->size; i++) {
                char *item_str = val_to_str_ptr(v.as.array->data[i]);
                if ((int)(strlen(res) + strlen(item_str) + 4) > cap) {
                    cap = cap * 2 + strlen(item_str);
                    res = realloc(res, cap);
                }
                strcat(res, item_str);
                free(item_str);
                if (i < v.as.array->size - 1) {
                    strcat(res, ", ");
                }
            }
            strcat(res, "]");
            return res;
        }
    }
    return strdup("");
}

void val_print(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        Value val = va_arg(args, Value);
        char *s = val_to_str_ptr(val);
        printf("%s", s);
        free(s);
        if (i < count - 1) {
            printf(" ");
        }
    }
    va_end(args);
    printf("\n");
}

Value val_input(Value prompt) {
    char *prompt_str = val_to_str_ptr(prompt);
    printf("%s", prompt_str);
    free(prompt_str);
    fflush(stdout);

    char buf[1024];
    if (fgets(buf, sizeof(buf), stdin)) {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
        }
        return val_string(buf);
    }
    return val_string("");
}

Value val_len(Value v) {
    if (v.type == VAL_STRING) {
        return val_number((double)strlen(v.as.string));
    } else if (v.type == VAL_ARRAY) {
        return val_number((double)v.as.array->size);
    }
    return val_number(0.0);
}

Value val_type(Value v) {
    switch (v.type) {
        case VAL_NULL: return val_string("null");
        case VAL_BOOL: return val_string("bool");
        case VAL_NUMBER: return val_string("number");
        case VAL_STRING: return val_string("string");
        case VAL_ARRAY: return val_string("array");
        case VAL_RAW_PTR: return val_string("raw_ptr");
    }
    return val_string("unknown");
}

Value val_int(Value v) {
    if (v.type == VAL_NUMBER) {
        return val_number(floor(v.as.number));
    } else if (v.type == VAL_STRING) {
        return val_number(atof(v.as.string));
    }
    return val_number(0.0);
}

Value val_float(Value v) {
    if (v.type == VAL_NUMBER) {
        return v;
    } else if (v.type == VAL_STRING) {
        return val_number(atof(v.as.string));
    }
    return val_number(0.0);
}

Value val_str(Value v) {
    char *s = val_to_str_ptr(v);
    Value res = val_string(s);
    free(s);
    return res;
}

Value val_abs(Value v) {
    if (v.type == VAL_NUMBER) {
        return val_number(fabs(v.as.number));
    }
    return val_number(0.0);
}

Value val_round(Value v) {
    if (v.type == VAL_NUMBER) {
        return val_number(round(v.as.number));
    }
    return val_number(0.0);
}

Value val_pow(Value x, Value y) {
    if (x.type == VAL_NUMBER && y.type == VAL_NUMBER) {
        return val_number(pow(x.as.number, y.as.number));
    }
    return val_number(0.0);
}

Value val_add(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_number(a.as.number + b.as.number);
    }
    char *s1 = val_to_str_ptr(a);
    char *s2 = val_to_str_ptr(b);
    char *res = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(res, s1);
    strcat(res, s2);
    Value val = val_string(res);
    free(s1);
    free(s2);
    free(res);
    return val;
}

Value val_sub(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_number(a.as.number - b.as.number);
    }
    return val_number(0.0);
}

Value val_mul(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_number(a.as.number * b.as.number);
    }
    return val_number(0.0);
}

Value val_div(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        if (b.as.number == 0.0) {
            fprintf(stderr, "Runtime Error: Division by zero\n");
            exit(1);
        }
        return val_number(a.as.number / b.as.number);
    }
    return val_number(0.0);
}

Value val_mod(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        if (b.as.number == 0.0) {
            fprintf(stderr, "Runtime Error: Modulo by zero\n");
            exit(1);
        }
        return val_number(fmod(a.as.number, b.as.number));
    }
    return val_number(0.0);
}

Value val_eq(Value a, Value b) {
    if (a.type != b.type) return val_bool(false);
    switch (a.type) {
        case VAL_NULL: return val_bool(true);
        case VAL_BOOL: return val_bool(a.as.boolean == b.as.boolean);
        case VAL_NUMBER: return val_bool(a.as.number == b.as.number);
        case VAL_STRING: return val_bool(strcmp(a.as.string, b.as.string) == 0);
        case VAL_ARRAY: return val_bool(a.as.array == b.as.array);
        case VAL_RAW_PTR: return val_bool(a.as.raw_ptr == b.as.raw_ptr);
    }
    return val_bool(false);
}

Value val_ne(Value a, Value b) {
    return val_bool(!val_to_bool(val_eq(a, b)));
}

Value val_lt(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_bool(a.as.number < b.as.number);
    }
    return val_bool(false);
}

Value val_gt(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_bool(a.as.number > b.as.number);
    }
    return val_bool(false);
}

Value val_le(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_bool(a.as.number <= b.as.number);
    }
    return val_bool(false);
}

Value val_ge(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_bool(a.as.number >= b.as.number);
    }
    return val_bool(false);
}

Value val_and(Value a, Value b) {
    return val_bool(val_to_bool(a) && val_to_bool(b));
}

Value val_or(Value a, Value b) {
    return val_bool(val_to_bool(a) || val_to_bool(b));
}

Value val_not(Value a) {
    return val_bool(!val_to_bool(a));
}

Value val_array_get(Value arr, Value index) {
    int idx = (int)index.as.number;
    if (arr.type == VAL_STRING) {
        int len = strlen(arr.as.string);
        if (idx < 0) idx += len;
        if (idx < 0 || idx >= len) {
            fprintf(stderr, "Runtime Error: String index out of bounds: %d\n", idx);
            exit(1);
        }
        char char_str[2] = { arr.as.string[idx], '\0' };
        return val_string(char_str);
    }
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: Object is not subscriptable\n");
        exit(1);
    }
    int len = arr.as.array->size;
    if (idx < 0) idx += len;
    if (idx < 0 || idx >= len) {
        fprintf(stderr, "Runtime Error: Array index out of bounds: %d\n", idx);
        exit(1);
    }
    return arr.as.array->data[idx];
}

Value val_array_set(Value arr, Value index, Value val) {
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: Object is not subscriptable\n");
        exit(1);
    }
    int idx = (int)index.as.number;
    int len = arr.as.array->size;
    if (idx < 0) idx += len;
    if (idx < 0 || idx >= len) {
        fprintf(stderr, "Runtime Error: Array index out of bounds: %d\n", idx);
        exit(1);
    }
    arr.as.array->data[idx] = val;
    return val;
}

Value val_array_slice(Value arr, Value start, Value end) {
    int len = 0;
    if (arr.type == VAL_STRING) {
        len = strlen(arr.as.string);
    } else if (arr.type == VAL_ARRAY) {
        len = arr.as.array->size;
    } else {
        fprintf(stderr, "Runtime Error: Object is not sliceable\n");
        exit(1);
    }

    int s_idx = 0;
    if (start.type == VAL_NULL) {
        s_idx = 0;
    } else {
        s_idx = (int)start.as.number;
        if (s_idx < 0) s_idx += len;
        if (s_idx < 0) s_idx = 0;
        if (s_idx > len) s_idx = len;
    }

    int e_idx = len;
    if (end.type == VAL_NULL) {
        e_idx = len;
    } else {
        e_idx = (int)end.as.number;
        if (e_idx < 0) e_idx += len;
        if (e_idx < 0) e_idx = 0;
        if (e_idx > len) e_idx = len;
    }

    if (arr.type == VAL_STRING) {
        if (s_idx >= e_idx) {
            return val_string("");
        }
        int slice_len = e_idx - s_idx;
        char *buf = malloc(slice_len + 1);
        memcpy(buf, arr.as.string + s_idx, slice_len);
        buf[slice_len] = '\0';
        Value res = val_string(buf);
        free(buf);
        return res;
    } else {
        if (s_idx >= e_idx) {
            return val_array_create(0);
        }
        int slice_len = e_idx - s_idx;
        Value res = val_array_create(slice_len);
        for (int i = 0; i < slice_len; i++) {
            res.as.array->data[i] = arr.as.array->data[s_idx + i];
        }
        return res;
    }
}

Value val_floordiv(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        if (b.as.number == 0.0) {
            fprintf(stderr, "Runtime Error: Division by zero\n");
            exit(1);
        }
        return val_number(floor(a.as.number / b.as.number));
    }
    return val_number(0.0);
}

Value val_array_append(Value arr, Value item) {
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: append() first argument must be an array\n");
        exit(1);
    }
    if (arr.as.array->size >= arr.as.array->capacity) {
        int new_cap = arr.as.array->capacity == 0 ? 8 : arr.as.array->capacity * 2;
        arr.as.array->data = realloc(arr.as.array->data, new_cap * sizeof(Value));
        arr.as.array->capacity = new_cap;
    }
    arr.as.array->data[arr.as.array->size++] = item;
    return arr;
}

Value val_array_pop(Value arr) {
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: pop() argument must be an array\n");
        exit(1);
    }
    if (arr.as.array->size == 0) {
        fprintf(stderr, "Runtime Error: pop() on empty array\n");
        exit(1);
    }
    return arr.as.array->data[--arr.as.array->size];
}

// ---- Raw Memory Operations Implementation ----
Value val_raw_malloc(Value size) {
    if (size.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: malloc size must be a number\n");
        exit(1);
    }
    Value v;
    v.type = VAL_RAW_PTR;
    v.as.raw_ptr = malloc((size_t)size.as.number);
    if (!v.as.raw_ptr && size.as.number > 0) {
        fprintf(stderr, "Out of memory in raw malloc\n");
        exit(1);
    }
    return v;
}

Value val_raw_realloc(Value ptr, Value size) {
    if (ptr.type != VAL_RAW_PTR && ptr.type != VAL_NULL) {
        fprintf(stderr, "Runtime Error: realloc first argument must be a raw pointer or null\n");
        exit(1);
    }
    if (size.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: realloc second argument must be a number\n");
        exit(1);
    }
    Value v;
    v.type = VAL_RAW_PTR;
    void *old_ptr = (ptr.type == VAL_RAW_PTR) ? ptr.as.raw_ptr : NULL;
    v.as.raw_ptr = realloc(old_ptr, (size_t)size.as.number);
    if (!v.as.raw_ptr && size.as.number > 0) {
        fprintf(stderr, "Out of memory in raw realloc\n");
        exit(1);
    }
    return v;
}

Value val_raw_free(Value ptr) {
    if (ptr.type == VAL_RAW_PTR) {
        if (ptr.as.raw_ptr) {
            free(ptr.as.raw_ptr);
        }
    }
    Value v;
    v.type = VAL_NULL;
    return v;
}

Value val_raw_ptr(void *p) {
    Value v;
    v.type = VAL_RAW_PTR;
    v.as.raw_ptr = p;
    return v;
}

Value val_arena_reset(WyrmArena *a) {
    arena_reset(a);
    Value v;
    v.type = VAL_NULL;
    return v;
}

Value wyrm_sys_args;

void val_init_sys_args(int argc, char *argv[]) {
    wyrm_sys_args = val_array_create(argc);
    for (int i = 0; i < argc; i++) {
        wyrm_sys_args.as.array->data[i] = val_string(argv[i]);
    }
}

Value val_sys_args() {
    return wyrm_sys_args;
}

Value val_read_file(Value path) {
    if (path.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: read_file path must be a string\n");
        exit(1);
    }
    FILE *f = fopen(path.as.string, "rb");
    if (!f) {
        return val_null();
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        fprintf(stderr, "Out of memory in read_file\n");
        exit(1);
    }
    size_t read_bytes = fread(buf, 1, size, f);
    buf[read_bytes] = '\0';
    fclose(f);

    Value res = val_string(buf);
    free(buf);
    return res;
}

Value val_write_file(Value path, Value content) {
    if (path.type != VAL_STRING || content.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: write_file arguments must be strings\n");
        exit(1);
    }
    FILE *f = fopen(path.as.string, "wb");
    if (!f) {
        return val_bool(false);
    }
    fwrite(content.as.string, 1, strlen(content.as.string), f);
    fclose(f);
    return val_bool(true);
}

Value val_exit(Value code) {
    if (code.type == VAL_NUMBER) {
        exit((int)code.as.number);
    }
    exit(0);
    return val_null();
}

Value val_system(Value cmd) {
    if (cmd.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: system command must be a string\n");
        exit(1);
    }
    int res = system(cmd.as.string);
    return val_number((double)res);
}

Value val_getenv(Value name) {
    if (name.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: getenv argument must be a string\n");
        exit(1);
    }
    char *res = getenv(name.as.string);
    if (!res) {
        return val_null();
    }
    return val_string(res);
}

// LLVM IR Wrapper functions
void llvm_val_null(Value *res) { *res = val_null(); }
void llvm_val_bool(Value *res, bool b) { *res = val_bool(b); }
void llvm_val_number(Value *res, double n) { *res = val_number(n); }
void llvm_val_string(Value *res, const char *s) { *res = val_string(s); }
void llvm_val_array_create(Value *res, int count) { *res = val_array_create(count); }
void llvm_val_array_init(Value *res, int count, ...) {
    va_list args;
    va_start(args, count);
    *res = val_array_create(count);
    for (int i = 0; i < count; i++) {
        res->as.array->data[i] = va_arg(args, Value);
    }
    va_end(args);
}
bool llvm_val_to_bool(Value *v) { return val_to_bool(*v); }

void llvm_val_print(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        Value *val = va_arg(args, Value*);
        char *s = val_to_str_ptr(*val);
        printf("%s", s);
        free(s);
        if (i < count - 1) {
            printf(" ");
        }
    }
    va_end(args);
    printf("\n");
}

void llvm_val_input(Value *res, Value *prompt) { *res = val_input(*prompt); }
void llvm_val_len(Value *res, Value *v) { *res = val_len(*v); }
void llvm_val_type(Value *res, Value *v) { *res = val_type(*v); }
void llvm_val_int(Value *res, Value *v) { *res = val_int(*v); }
void llvm_val_float(Value *res, Value *v) { *res = val_float(*v); }
void llvm_val_str(Value *res, Value *v) { *res = val_str(*v); }
void llvm_val_abs(Value *res, Value *v) { *res = val_abs(*v); }
void llvm_val_round(Value *res, Value *v) { *res = val_round(*v); }
void llvm_val_pow(Value *res, Value *x, Value *y) { *res = val_pow(*x, *y); }

void llvm_val_add(Value *res, Value *a, Value *b) { *res = val_add(*a, *b); }
void llvm_val_sub(Value *res, Value *a, Value *b) { *res = val_sub(*a, *b); }
void llvm_val_mul(Value *res, Value *a, Value *b) { *res = val_mul(*a, *b); }
void llvm_val_div(Value *res, Value *a, Value *b) { *res = val_div(*a, *b); }
void llvm_val_mod(Value *res, Value *a, Value *b) { *res = val_mod(*a, *b); }
void llvm_val_floordiv(Value *res, Value *a, Value *b) { *res = val_floordiv(*a, *b); }
void llvm_val_eq(Value *res, Value *a, Value *b) { *res = val_eq(*a, *b); }
void llvm_val_ne(Value *res, Value *a, Value *b) { *res = val_ne(*a, *b); }
void llvm_val_lt(Value *res, Value *a, Value *b) { *res = val_lt(*a, *b); }
void llvm_val_gt(Value *res, Value *a, Value *b) { *res = val_gt(*a, *b); }
void llvm_val_le(Value *res, Value *a, Value *b) { *res = val_le(*a, *b); }
void llvm_val_ge(Value *res, Value *a, Value *b) { *res = val_ge(*a, *b); }
void llvm_val_and(Value *res, Value *a, Value *b) { *res = val_and(*a, *b); }
void llvm_val_or(Value *res, Value *a, Value *b) { *res = val_or(*a, *b); }
void llvm_val_not(Value *res, Value *a) { *res = val_not(*a); }

void llvm_val_array_get(Value *res, Value *arr, Value *index) { *res = val_array_get(*arr, *index); }
void llvm_val_array_set(Value *arr, Value *index, Value *val) { val_array_set(*arr, *index, *val); }
void llvm_val_array_append(Value *res, Value *arr, Value *item) { *res = val_array_append(*arr, *item); }
void llvm_val_array_pop(Value *res, Value *arr) { *res = val_array_pop(*arr); }
void llvm_val_array_slice(Value *res, Value *arr, Value *start, Value *end) { *res = val_array_slice(*arr, *start, *end); }

void llvm_val_raw_malloc(Value *res, Value *size) { *res = val_raw_malloc(*size); }
void llvm_val_raw_realloc(Value *res, Value *ptr, Value *size) { *res = val_raw_realloc(*ptr, *size); }
void llvm_val_raw_free(Value *res, Value *ptr) { *res = val_raw_free(*ptr); }
void llvm_val_raw_ptr(Value *res, void *p) { *res = val_raw_ptr(p); }
void llvm_val_arena_reset(Value *res, WyrmArena *a) { *res = val_arena_reset(a); }

void llvm_val_read_file(Value *res, Value *path) { *res = val_read_file(*path); }
void llvm_val_write_file(Value *res, Value *path, Value *content) { *res = val_write_file(*path, *content); }
void llvm_val_exit(Value *res, Value *code) { *res = val_exit(*code); }
void llvm_val_system(Value *res, Value *cmd) { *res = val_system(*cmd); }

// String ops wrappers
void llvm_val_split(Value *res, Value *a, Value *b) { *res = val_split(*a, *b); }
void llvm_val_join(Value *res, Value *a, Value *b) { *res = val_join(*a, *b); }
void llvm_val_trim(Value *res, Value *a) { *res = val_trim(*a); }
void llvm_val_upper(Value *res, Value *a) { *res = val_upper(*a); }
void llvm_val_lower(Value *res, Value *a) { *res = val_lower(*a); }
void llvm_val_contains(Value *res, Value *a, Value *b) { *res = val_contains(*a, *b); }
void llvm_val_replace(Value *res, Value *a, Value *b, Value *c) { *res = val_replace(*a, *b, *c); }
void llvm_val_starts_with(Value *res, Value *a, Value *b) { *res = val_starts_with(*a, *b); }
void llvm_val_ends_with(Value *res, Value *a, Value *b) { *res = val_ends_with(*a, *b); }
void llvm_val_char_at(Value *res, Value *a, Value *b) { *res = val_char_at(*a, *b); }
void llvm_val_ord_val(Value *res, Value *a) { *res = val_ord_val(*a); }
void llvm_val_chr_val(Value *res, Value *a) { *res = val_chr_val(*a); }
void llvm_val_to_bytes(Value *res, Value *a) { *res = val_to_bytes(*a); }
void llvm_val_from_bytes(Value *res, Value *a) { *res = val_from_bytes(*a); }
void llvm_val_copy(Value *dest, Value *src) { *dest = *src; }

WyrmArena* val_arena_create_wrapper(Value *size) {
    if (size->type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: arena size must be a number\n");
        exit(1);
    }
    return arena_create((size_t)size->as.number);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- Arena Allocator Implementation ----

WyrmArena *arena_create(size_t cap) {
    WyrmArena *a = (WyrmArena*)malloc(sizeof(WyrmArena));
    if (!a) { fprintf(stderr, "Out of memory\n"); exit(1); }
    a->buf   = (char*)malloc(cap);
    if (!a->buf) { fprintf(stderr, "Out of memory\n"); exit(1); }
    a->cap   = cap;
    a->used  = 0;
    a->freed = 0;
    return a;
}

void *arena_alloc(WyrmArena *a, size_t sz) {
    if (!a || a->freed) {
        fprintf(stderr, "Runtime Error: arena_alloc() on freed/null arena\n");
        exit(1);
    }
    // Align to 8 bytes
    size_t aligned = (sz + 7u) & ~7u;
    if (a->used + aligned > a->cap) {
        fprintf(stderr, "Runtime Error: arena out of space (cap=%zu used=%zu requested=%zu)\n",
                a->cap, a->used, aligned);
        exit(1);
    }
    void *ptr = a->buf + a->used;
    a->used += aligned;
    return ptr;
}

void arena_reset(WyrmArena *a) {
    if (!a) return;
    a->used  = 0;
    a->freed = 1;  // prevent further alloc() without reinit
}

void arena_destroy(WyrmArena *a) {
    if (!a) return;
    free(a->buf);
    free(a);
}


Value val_split(Value s, Value sep) {
    if (s.type != VAL_STRING || sep.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: split() arguments must be strings\n");
        exit(1);
    }
    const char *str = s.as.string;
    const char *delim = sep.as.string;
    size_t delim_len = strlen(delim);
    
    int count = 0;
    if (delim_len == 0) {
        count = (int)strlen(str);
    } else {
        const char *temp = str;
        while ((temp = strstr(temp, delim)) != NULL) {
            count++;
            temp += delim_len;
        }
        count++;
    }
    
    Value arr = val_array_create(count);
    if (delim_len == 0) {
        for (int i = 0; i < count; i++) {
            char buf[2] = { str[i], '\0' };
            arr.as.array->data[i] = val_string(buf);
        }
    } else {
        const char *start = str;
        const char *match;
        int idx = 0;
        while ((match = strstr(start, delim)) != NULL) {
            size_t part_len = (size_t)(match - start);
            char *buf = malloc(part_len + 1);
            if (!buf) { fprintf(stderr, "Out of memory\n"); exit(1); }
            memcpy(buf, start, part_len);
            buf[part_len] = '\0';
            arr.as.array->data[idx++] = val_string(buf);
            free(buf);
            start = match + delim_len;
        }
        arr.as.array->data[idx] = val_string(start);
    }
    return arr;
}

Value val_join(Value sep, Value lst) {
    if (sep.type != VAL_STRING || lst.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: join() arguments must be separator string and array\n");
        exit(1);
    }
    int count = lst.as.array->size;
    if (count == 0) {
        return val_string("");
    }
    
    char **strs = malloc((size_t)count * sizeof(char*));
    if (!strs) { fprintf(stderr, "Out of memory\n"); exit(1); }
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        strs[i] = val_to_str_ptr(lst.as.array->data[i]);
        total_len += strlen(strs[i]);
    }
    
    size_t sep_len = strlen(sep.as.string);
    total_len += sep_len * (size_t)(count - 1);
    
    char *res = malloc(total_len + 1);
    if (!res) { fprintf(stderr, "Out of memory\n"); exit(1); }
    res[0] = '\0';
    
    for (int i = 0; i < count; i++) {
        strcat(res, strs[i]);
        free(strs[i]);
        if (i < count - 1) {
            strcat(res, sep.as.string);
        }
    }
    free(strs);
    
    Value val = val_string(res);
    free(res);
    return val;
}

Value val_trim(Value s) {
    if (s.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: trim() argument must be a string\n");
        exit(1);
    }
    const char *start = s.as.string;
    while (*start && ((unsigned char)*start <= ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }
    size_t len = strlen(start);
    while (len > 0 && ((unsigned char)start[len - 1] <= ' ' || start[len - 1] == '\t' || start[len - 1] == '\n' || start[len - 1] == '\r')) {
        len--;
    }
    char *buf = malloc(len + 1);
    if (!buf) { fprintf(stderr, "Out of memory\n"); exit(1); }
    memcpy(buf, start, len);
    buf[len] = '\0';
    Value val = val_string(buf);
    free(buf);
    return val;
}

Value val_upper(Value s) {
    if (s.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: upper() argument must be a string\n");
        exit(1);
    }
    size_t len = strlen(s.as.string);
    char *buf = malloc(len + 1);
    if (!buf) { fprintf(stderr, "Out of memory\n"); exit(1); }
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s.as.string[i];
        if (c >= 'a' && c <= 'z') {
            buf[i] = (char)(c - 32);
        } else {
            buf[i] = (char)c;
        }
    }
    buf[len] = '\0';
    Value val = val_string(buf);
    free(buf);
    return val;
}

Value val_lower(Value s) {
    if (s.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: lower() argument must be a string\n");
        exit(1);
    }
    size_t len = strlen(s.as.string);
    char *buf = malloc(len + 1);
    if (!buf) { fprintf(stderr, "Out of memory\n"); exit(1); }
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s.as.string[i];
        if (c >= 'A' && c <= 'Z') {
            buf[i] = (char)(c + 32);
        } else {
            buf[i] = (char)c;
        }
    }
    buf[len] = '\0';
    Value val = val_string(buf);
    free(buf);
    return val;
}

Value val_contains(Value s, Value sub) {
    if (s.type != VAL_STRING || sub.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: contains() arguments must be strings\n");
        exit(1);
    }
    return val_bool(strstr(s.as.string, sub.as.string) != NULL);
}

Value val_replace(Value s, Value old, Value sub_new) {
    if (s.type != VAL_STRING || old.type != VAL_STRING || sub_new.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: replace() arguments must be strings\n");
        exit(1);
    }
    const char *str = s.as.string;
    const char *old_str = old.as.string;
    const char *new_str = sub_new.as.string;
    size_t old_len = strlen(old_str);
    size_t new_len = strlen(new_str);
    
    if (old_len == 0) {
        return val_string(str);
    }
    
    int count = 0;
    const char *temp = str;
    while ((temp = strstr(temp, old_str)) != NULL) {
        count++;
        temp += old_len;
    }
    
    size_t total_len = strlen(str) + (new_len - old_len) * (size_t)count;
    char *res = malloc(total_len + 1);
    if (!res) { fprintf(stderr, "Out of memory\n"); exit(1); }
    
    const char *start = str;
    const char *match;
    char *dest = res;
    while ((match = strstr(start, old_str)) != NULL) {
        size_t copy_len = (size_t)(match - start);
        memcpy(dest, start, copy_len);
        dest += copy_len;
        memcpy(dest, new_str, new_len);
        dest += new_len;
        start = match + old_len;
    }
    strcpy(dest, start);
    
    Value val = val_string(res);
    free(res);
    return val;
}

Value val_starts_with(Value s, Value prefix) {
    if (s.type != VAL_STRING || prefix.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: starts_with() arguments must be strings\n");
        exit(1);
    }
    size_t prefix_len = strlen(prefix.as.string);
    if (strlen(s.as.string) < prefix_len) {
        return val_bool(false);
    }
    return val_bool(strncmp(s.as.string, prefix.as.string, prefix_len) == 0);
}

Value val_ends_with(Value s, Value suffix) {
    if (s.type != VAL_STRING || suffix.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: ends_with() arguments must be strings\n");
        exit(1);
    }
    size_t s_len = strlen(s.as.string);
    size_t suffix_len = strlen(suffix.as.string);
    if (s_len < suffix_len) {
        return val_bool(false);
    }
    return val_bool(strcmp(s.as.string + s_len - suffix_len, suffix.as.string) == 0);
}

Value val_char_at(Value s, Value idx) {
    if (s.type != VAL_STRING || idx.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: char_at() arguments must be string and index\n");
        exit(1);
    }
    int len = (int)strlen(s.as.string);
    int i = (int)idx.as.number;
    if (i < 0) i += len;
    if (i < 0 || i >= len) {
        fprintf(stderr, "Runtime Error: char_at() index out of bounds: %d\n", i);
        exit(1);
    }
    char buf[2] = { s.as.string[i], '\0' };
    return val_string(buf);
}

Value val_ord_val(Value c) {
    if (c.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: ord_val() argument must be a string\n");
        exit(1);
    }
    if (strlen(c.as.string) != 1) {
        fprintf(stderr, "Runtime Error: ord_val() argument must be a single character\n");
        exit(1);
    }
    return val_number((double)((unsigned char)c.as.string[0]));
}

Value val_chr_val(Value n) {
    if (n.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: chr_val() argument must be a number\n");
        exit(1);
    }
    int val = (int)n.as.number;
    if (val < 0 || val > 255) {
        fprintf(stderr, "Runtime Error: chr_val() argument out of valid single-byte range: %d\n", val);
        exit(1);
    }
    char buf[2] = { (char)val, '\0' };
    return val_string(buf);
}

Value val_to_bytes(Value s) {
    if (s.type != VAL_STRING) {
        fprintf(stderr, "Runtime Error: to_bytes() argument must be a string\n");
        exit(1);
    }
    int len = (int)strlen(s.as.string);
    Value arr = val_array_create(len);
    for (int i = 0; i < len; i++) {
        arr.as.array->data[i] = val_number((double)((unsigned char)s.as.string[i]));
    }
    return arr;
}

Value val_from_bytes(Value lst) {
    if (lst.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: from_bytes() argument must be an array\n");
        exit(1);
    }
    int len = lst.as.array->size;
    char *buf = malloc((size_t)len + 1);
    if (!buf) { fprintf(stderr, "Out of memory\n"); exit(1); }
    for (int i = 0; i < len; i++) {
        Value val = lst.as.array->data[i];
        if (val.type != VAL_NUMBER) {
            fprintf(stderr, "Runtime Error: from_bytes() array element must be a number\n");
            free(buf);
            exit(1);
        }
        int code = (int)val.as.number;
        if (code < 0 || code > 255) {
            fprintf(stderr, "Runtime Error: from_bytes() byte value out of range: %d\n", code);
            free(buf);
            exit(1);
        }
        buf[i] = (char)code;
    }
    buf[len] = '\0';
    Value val = val_string(buf);
    free(buf);
    return val;
}



// Forward declarations of user functions
Value wyrm_fn_is_alpha(Value wyrm_var_c);
Value wyrm_fn_is_digit(Value wyrm_var_c);
Value wyrm_fn_is_alphanum(Value wyrm_var_c);
Value wyrm_fn_is_space(Value wyrm_var_c);
Value wyrm_fn_tokenize(Value wyrm_var_source);
Value wyrm_fn_contains_val(Value wyrm_var_lst, Value wyrm_var_val);
Value wyrm_fn_current();
Value wyrm_fn_cur_type();
Value wyrm_fn_cur_val();
Value wyrm_fn_peek_val();
Value wyrm_fn_advance();
Value wyrm_fn_expect(Value wyrm_var_type, Value wyrm_var_val);
Value wyrm_fn_parse_tokens(Value wyrm_var_tokens);
Value wyrm_fn_parse_statement();
Value wyrm_fn_parse_expression();
Value wyrm_fn_parse_catch();
Value wyrm_fn_parse_logical_or();
Value wyrm_fn_parse_logical_and();
Value wyrm_fn_parse_equality();
Value wyrm_fn_parse_comparison();
Value wyrm_fn_parse_addition_subtraction();
Value wyrm_fn_parse_multiplication_division();
Value wyrm_fn_parse_power();
Value wyrm_fn_parse_unary();
Value wyrm_fn_parse_primary();
Value wyrm_fn_gen_temp();
Value wyrm_fn_emit_inst(Value wyrm_var_inst);
Value wyrm_fn_llvm_escape_string(Value wyrm_var_s);
Value wyrm_fn_push_scope();
Value wyrm_fn_pop_scope();
Value wyrm_fn_gen_all_cleanups();
Value wyrm_fn_has_dot(Value wyrm_var_s);
Value wyrm_fn_gen_expr(Value wyrm_var_node);
Value wyrm_fn_gen_statement(Value wyrm_var_stmt);
Value wyrm_fn_emit_label(Value wyrm_var_label);
Value wyrm_fn_check_expr(Value wyrm_var_expr, Value wyrm_var_error_funcs, Value wyrm_var_in_try_stmt, Value wyrm_var_in_try_expr, Value wyrm_var_in_catch_expr, Value wyrm_var_func_returns_error);
Value wyrm_fn_check_stmt(Value wyrm_var_stmt, Value wyrm_var_error_funcs, Value wyrm_var_in_try_stmt, Value wyrm_var_func_returns_error);
Value wyrm_fn_check_errors(Value wyrm_var_ast);
Value wyrm_fn_transpile_llvm(Value wyrm_var_ast);
Value wyrm_fn_parse_file(Value wyrm_var_path, Value wyrm_var_processed_files);

// Global variable declarations
Value wyrm_var_token_pos;
Value wyrm_var_g_tokens;
Value wyrm_var_temp_count;
Value wyrm_var_g_allocas;
Value wyrm_var_g_main_allocas;
Value wyrm_var_g_insts;
Value wyrm_var_g_llvm_globals;
Value wyrm_var_g_llvm_decls;
Value wyrm_var_g_llvm_funcs;
Value wyrm_var_g_llvm_main;
Value wyrm_var_g_scopes;
Value wyrm_var_g_catch_labels;
Value wyrm_var_g_catch_vars;
Value wyrm_var_in_function;
Value wyrm_var_in_unsafe;
Value wyrm_var_has_main_def;
Value wyrm_var_declared_globals;
Value wyrm_var_declared_locals;
Value wyrm_var_constants;

// Function implementations

Value wyrm_fn_is_alpha(Value wyrm_var_c) {
    if (val_to_bool(val_eq(wyrm_var_c, val_string("")))) {
    return val_bool(false);
    }
    Value wyrm_var_code = val_ord_val(wyrm_var_c);
    return val_or(val_or(val_and(val_ge(wyrm_var_code, val_number(65)), val_le(wyrm_var_code, val_number(90))), val_and(val_ge(wyrm_var_code, val_number(97)), val_le(wyrm_var_code, val_number(122)))), val_eq(wyrm_var_c, val_string("_")));
    return val_null();
}

Value wyrm_fn_is_digit(Value wyrm_var_c) {
    if (val_to_bool(val_eq(wyrm_var_c, val_string("")))) {
    return val_bool(false);
    }
    Value wyrm_var_code = val_ord_val(wyrm_var_c);
    return val_and(val_ge(wyrm_var_code, val_number(48)), val_le(wyrm_var_code, val_number(57)));
    return val_null();
}

Value wyrm_fn_is_alphanum(Value wyrm_var_c) {
    return val_or(wyrm_fn_is_alpha(wyrm_var_c), wyrm_fn_is_digit(wyrm_var_c));
    return val_null();
}

Value wyrm_fn_is_space(Value wyrm_var_c) {
    return val_or(val_or(val_eq(wyrm_var_c, val_string(" ")), val_eq(wyrm_var_c, val_string("\t"))), val_eq(wyrm_var_c, val_string("\r")));
    return val_null();
}

Value wyrm_fn_tokenize(Value wyrm_var_source) {
    Value wyrm_var_tokens = val_array_init(0);
    Value wyrm_var_pos = val_number(0);
    Value wyrm_var_len_src = val_len(wyrm_var_source);
    Value wyrm_var_line = val_number(1);
    Value wyrm_var_col = val_number(1);
    do {
    if (val_to_bool(val_ge(wyrm_var_pos, wyrm_var_len_src))) {
    break;
    }
    Value wyrm_var_c = val_char_at(wyrm_var_source, wyrm_var_pos);
    if (val_to_bool(val_eq(wyrm_var_c, val_string("\n")))) {
    val_array_append(wyrm_var_tokens, val_array_init(4, val_string("NEWLINE"), val_string("\n"), wyrm_var_line, wyrm_var_col));
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_line = val_add(wyrm_var_line, val_number(1));
    wyrm_var_col = val_number(1);
    }
    else if (val_to_bool(wyrm_fn_is_space(wyrm_var_c))) {
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_col = val_add(wyrm_var_col, val_number(1));
    }
    else {
    Value wyrm_var_is_comment = val_bool(false);
    if (val_to_bool(val_eq(wyrm_var_c, val_string("/")))) {
    if (val_to_bool(val_lt(val_add(wyrm_var_pos, val_number(1)), wyrm_var_len_src))) {
    if (val_to_bool(val_eq(val_char_at(wyrm_var_source, val_add(wyrm_var_pos, val_number(1))), val_string("/")))) {
    wyrm_var_is_comment = val_bool(true);
    }
    }
    }
    if (val_to_bool(wyrm_var_is_comment)) {
    do {
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_col = val_add(wyrm_var_col, val_number(1));
    if (val_to_bool(val_ge(wyrm_var_pos, wyrm_var_len_src))) {
    break;
    }
    wyrm_var_c = val_char_at(wyrm_var_source, wyrm_var_pos);
    } while (!val_to_bool(val_or(val_eq(wyrm_var_c, val_string("\n")), val_ge(wyrm_var_pos, wyrm_var_len_src))));
    }
    else {
    if (val_to_bool(wyrm_fn_is_alpha(wyrm_var_c))) {
    Value wyrm_var_start_col = wyrm_var_col;
    Value wyrm_var_val = val_string("");
    do {
    wyrm_var_val = val_add(wyrm_var_val, wyrm_var_c);
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_col = val_add(wyrm_var_col, val_number(1));
    if (val_to_bool(val_ge(wyrm_var_pos, wyrm_var_len_src))) {
    break;
    }
    wyrm_var_c = val_char_at(wyrm_var_source, wyrm_var_pos);
    } while (!val_to_bool(val_or(val_not(wyrm_fn_is_alphanum(wyrm_var_c)), val_ge(wyrm_var_pos, wyrm_var_len_src))));
    Value wyrm_var_type = val_string("IDENTIFIER");
    if (val_to_bool(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_eq(wyrm_var_val, val_string("use")), val_eq(wyrm_var_val, val_string("fn"))), val_eq(wyrm_var_val, val_string("var"))), val_eq(wyrm_var_val, val_string("dec"))), val_eq(wyrm_var_val, val_string("owned"))), val_eq(wyrm_var_val, val_string("arena"))), val_eq(wyrm_var_val, val_string("if"))), val_eq(wyrm_var_val, val_string("elif"))), val_eq(wyrm_var_val, val_string("else"))), val_eq(wyrm_var_val, val_string("repeat"))), val_eq(wyrm_var_val, val_string("til"))), val_eq(wyrm_var_val, val_string("unsafe"))), val_eq(wyrm_var_val, val_string("return"))), val_eq(wyrm_var_val, val_string("break"))), val_eq(wyrm_var_val, val_string("continue"))), val_eq(wyrm_var_val, val_string("print"))), val_eq(wyrm_var_val, val_string("and"))), val_eq(wyrm_var_val, val_string("or"))), val_eq(wyrm_var_val, val_string("not"))), val_eq(wyrm_var_val, val_string("true"))), val_eq(wyrm_var_val, val_string("false"))), val_eq(wyrm_var_val, val_string("null"))), val_eq(wyrm_var_val, val_string("try"))), val_eq(wyrm_var_val, val_string("catch"))))) {
    wyrm_var_type = val_string("KEYWORD");
    }
    val_array_append(wyrm_var_tokens, val_array_init(4, wyrm_var_type, wyrm_var_val, wyrm_var_line, wyrm_var_start_col));
    }
    else if (val_to_bool(wyrm_fn_is_digit(wyrm_var_c))) {
    Value wyrm_var_start_col = wyrm_var_col;
    Value wyrm_var_val = val_string("");
    Value wyrm_var_has_dot = val_bool(false);
    do {
    if (val_to_bool(val_eq(wyrm_var_c, val_string(".")))) {
    wyrm_var_has_dot = val_bool(true);
    }
    wyrm_var_val = val_add(wyrm_var_val, wyrm_var_c);
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_col = val_add(wyrm_var_col, val_number(1));
    if (val_to_bool(val_ge(wyrm_var_pos, wyrm_var_len_src))) {
    break;
    }
    wyrm_var_c = val_char_at(wyrm_var_source, wyrm_var_pos);
    } while (!val_to_bool(val_or(val_and(val_not(wyrm_fn_is_digit(wyrm_var_c)), val_ne(wyrm_var_c, val_string("."))), val_ge(wyrm_var_pos, wyrm_var_len_src))));
    val_array_append(wyrm_var_tokens, val_array_init(4, val_string("NUMBER"), wyrm_var_val, wyrm_var_line, wyrm_var_start_col));
    }
    else if (val_to_bool(val_eq(wyrm_var_c, val_string("\"")))) {
    Value wyrm_var_start_col = wyrm_var_col;
    Value wyrm_var_val = val_string("");
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_col = val_add(wyrm_var_col, val_number(1));
    Value wyrm_var_closed = val_bool(false);
    do {
    if (val_to_bool(val_ge(wyrm_var_pos, wyrm_var_len_src))) {
    break;
    }
    wyrm_var_c = val_char_at(wyrm_var_source, wyrm_var_pos);
    if (val_to_bool(val_eq(wyrm_var_c, val_string("\"")))) {
    wyrm_var_closed = val_bool(true);
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_col = val_add(wyrm_var_col, val_number(1));
    break;
    }
    if (val_to_bool(val_and(val_eq(wyrm_var_c, val_string("\\")), val_lt(val_add(wyrm_var_pos, val_number(1)), wyrm_var_len_src)))) {
    Value wyrm_var_next_c = val_char_at(wyrm_var_source, val_add(wyrm_var_pos, val_number(1)));
    if (val_to_bool(val_eq(wyrm_var_next_c, val_string("n")))) {
    wyrm_var_val = val_add(wyrm_var_val, val_string("\n"));
    }
    else if (val_to_bool(val_eq(wyrm_var_next_c, val_string("t")))) {
    wyrm_var_val = val_add(wyrm_var_val, val_string("\t"));
    }
    else if (val_to_bool(val_eq(wyrm_var_next_c, val_string("r")))) {
    wyrm_var_val = val_add(wyrm_var_val, val_string("\r"));
    }
    else if (val_to_bool(val_eq(wyrm_var_next_c, val_string("\\")))) {
    wyrm_var_val = val_add(wyrm_var_val, val_string("\\"));
    }
    else if (val_to_bool(val_eq(wyrm_var_next_c, val_string("\"")))) {
    wyrm_var_val = val_add(wyrm_var_val, val_string("\""));
    }
    else {
    wyrm_var_val = val_add(val_add(wyrm_var_val, val_string("\\")), wyrm_var_next_c);
    }
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(2));
    wyrm_var_col = val_add(wyrm_var_col, val_number(2));
    }
    else {
    wyrm_var_val = val_add(wyrm_var_val, wyrm_var_c);
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_col = val_add(wyrm_var_col, val_number(1));
    }
    } while (!val_to_bool(val_ge(wyrm_var_pos, wyrm_var_len_src)));
    if (val_to_bool(val_not(wyrm_var_closed))) {
    val_print(4, val_string("Lexer Error: Unterminated string literal at line"), wyrm_var_line, val_string("col"), wyrm_var_start_col);
    val_exit(val_number(1));
    }
    val_array_append(wyrm_var_tokens, val_array_init(4, val_string("STRING"), wyrm_var_val, wyrm_var_line, wyrm_var_start_col));
    }
    else {
    Value wyrm_var_start_col = wyrm_var_col;
    Value wyrm_var_val2 = val_string("");
    if (val_to_bool(val_lt(val_add(wyrm_var_pos, val_number(1)), wyrm_var_len_src))) {
    wyrm_var_val2 = val_add(wyrm_var_c, val_char_at(wyrm_var_source, val_add(wyrm_var_pos, val_number(1))));
    }
    if (val_to_bool(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_eq(wyrm_var_val2, val_string("==")), val_eq(wyrm_var_val2, val_string("!="))), val_eq(wyrm_var_val2, val_string("<="))), val_eq(wyrm_var_val2, val_string(">="))), val_eq(wyrm_var_val2, val_string("&&"))), val_eq(wyrm_var_val2, val_string("||"))), val_eq(wyrm_var_val2, val_string("**"))), val_eq(wyrm_var_val2, val_string("//"))), val_eq(wyrm_var_val2, val_string("+="))), val_eq(wyrm_var_val2, val_string("-="))), val_eq(wyrm_var_val2, val_string("*="))), val_eq(wyrm_var_val2, val_string("/="))), val_eq(wyrm_var_val2, val_string("->"))))) {
    val_array_append(wyrm_var_tokens, val_array_init(4, val_string("OPERATOR"), wyrm_var_val2, wyrm_var_line, wyrm_var_start_col));
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(2));
    wyrm_var_col = val_add(wyrm_var_col, val_number(2));
    }
    else {
    Value wyrm_var_type = val_string("DELIMITER");
    if (val_to_bool(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_eq(wyrm_var_c, val_string("+")), val_eq(wyrm_var_c, val_string("-"))), val_eq(wyrm_var_c, val_string("*"))), val_eq(wyrm_var_c, val_string("/"))), val_eq(wyrm_var_c, val_string("%"))), val_eq(wyrm_var_c, val_string("="))), val_eq(wyrm_var_c, val_string("!"))), val_eq(wyrm_var_c, val_string("<"))), val_eq(wyrm_var_c, val_string(">"))))) {
    wyrm_var_type = val_string("OPERATOR");
    }
    val_array_append(wyrm_var_tokens, val_array_init(4, wyrm_var_type, wyrm_var_c, wyrm_var_line, wyrm_var_start_col));
    wyrm_var_pos = val_add(wyrm_var_pos, val_number(1));
    wyrm_var_col = val_add(wyrm_var_col, val_number(1));
    }
    }
    }
    }
    } while (!val_to_bool(val_ge(wyrm_var_pos, wyrm_var_len_src)));
    val_array_append(wyrm_var_tokens, val_array_init(4, val_string("EOF"), val_string(""), wyrm_var_line, wyrm_var_col));
    return wyrm_var_tokens;
    return val_null();
}

Value wyrm_fn_contains_val(Value wyrm_var_lst, Value wyrm_var_val) {
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_l = val_len(wyrm_var_lst);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_lst, wyrm_var_i), wyrm_var_val))) {
    return val_bool(true);
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    return val_bool(false);
    return val_null();
}

Value wyrm_fn_current() {
    Value wyrm_var_tokens_list = val_array_get(wyrm_var_g_tokens, val_number(0));
    Value wyrm_var_pos = val_array_get(wyrm_var_token_pos, val_number(0));
    return val_array_get(wyrm_var_tokens_list, wyrm_var_pos);
    return val_null();
}

Value wyrm_fn_cur_type() {
    Value wyrm_var_tok = wyrm_fn_current();
    return val_array_get(wyrm_var_tok, val_number(0));
    return val_null();
}

Value wyrm_fn_cur_val() {
    Value wyrm_var_tok = wyrm_fn_current();
    return val_array_get(wyrm_var_tok, val_number(1));
    return val_null();
}

Value wyrm_fn_peek_val() {
    Value wyrm_var_tokens_list = val_array_get(wyrm_var_g_tokens, val_number(0));
    Value wyrm_var_pos = val_add(val_array_get(wyrm_var_token_pos, val_number(0)), val_number(1));
    if (val_to_bool(val_lt(wyrm_var_pos, val_len(wyrm_var_tokens_list)))) {
    Value wyrm_var_tok = val_array_get(wyrm_var_tokens_list, wyrm_var_pos);
    return val_array_get(wyrm_var_tok, val_number(1));
    }
    return val_string("");
    return val_null();
}

Value wyrm_fn_advance() {
    val_array_set(wyrm_var_token_pos, val_number(0), val_add(val_array_get(wyrm_var_token_pos, val_number(0)), val_number(1)));
    return val_null();
}

Value wyrm_fn_expect(Value wyrm_var_type, Value wyrm_var_val) {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_or(val_ne(val_array_get(wyrm_var_tok, val_number(0)), wyrm_var_type), val_ne(val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_val)))) {
    val_print(8, val_string("Parser Error: Expected token"), wyrm_var_type, val_add(val_add(val_string("'"), wyrm_var_val), val_string("' but got")), val_array_get(wyrm_var_tok, val_number(0)), val_add(val_add(val_string("'"), val_array_get(wyrm_var_tok, val_number(1))), val_string("' at line")), val_array_get(wyrm_var_tok, val_number(2)), val_string("col"), val_array_get(wyrm_var_tok, val_number(3)));
    val_exit(val_number(1));
    }
    wyrm_fn_advance();
    return val_null();
}

Value wyrm_fn_parse_tokens(Value wyrm_var_tokens) {
    val_array_set(wyrm_var_g_tokens, val_number(0), wyrm_var_tokens);
    val_print(2, val_string("DEBUG parse_tokens(): assigned g_tokens len ="), val_len(val_array_get(wyrm_var_g_tokens, val_number(0))));
    val_array_set(wyrm_var_token_pos, val_number(0), val_number(0));
    Value wyrm_var_statements = val_array_init(0);
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("EOF")))) {
    break;
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_statements, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_statements;
    return val_null();
}

Value wyrm_fn_parse_statement() {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")))) {
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("use")))) {
    wyrm_fn_advance();
    Value wyrm_var_path = wyrm_fn_cur_val();
    wyrm_fn_advance();
    return val_array_init(2, val_string("Use"), wyrm_var_path);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("fn")))) {
    wyrm_fn_advance();
    Value wyrm_var_name = wyrm_fn_cur_val();
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("("));
    Value wyrm_var_params = val_array_init(0);
    if (val_to_bool(val_ne(wyrm_fn_cur_val(), val_string(")")))) {
    do {
    val_array_append(wyrm_var_params, wyrm_fn_cur_val());
    wyrm_fn_advance();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(",")))) {
    wyrm_fn_advance();
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    }
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    Value wyrm_var_returns_error = val_bool(false);
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("->")))) {
    wyrm_fn_advance();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("!")))) {
    wyrm_var_returns_error = val_bool(true);
    wyrm_fn_advance();
    }
    wyrm_fn_advance();
    }
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    Value wyrm_var_body = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_body, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    return val_array_init(5, val_string("FuncDef"), wyrm_var_name, wyrm_var_params, wyrm_var_body, wyrm_var_returns_error);
    }
    if (val_to_bool(val_or(val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("var")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("dec"))), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("owned"))))) {
    Value wyrm_var_type = val_array_get(wyrm_var_tok, val_number(1));
    wyrm_fn_advance();
    Value wyrm_var_name = wyrm_fn_cur_val();
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("OPERATOR"), val_string("="));
    Value wyrm_var_val = wyrm_fn_parse_expression();
    return val_array_init(4, val_string("VarDecl"), wyrm_var_type, wyrm_var_name, wyrm_var_val);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("arena")))) {
    wyrm_fn_advance();
    Value wyrm_var_name = wyrm_fn_cur_val();
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("("));
    Value wyrm_var_size = wyrm_fn_parse_expression();
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    return val_array_init(3, val_string("Arena"), wyrm_var_name, wyrm_var_size);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("if")))) {
    wyrm_fn_advance();
    Value wyrm_var_cond = wyrm_fn_parse_expression();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    Value wyrm_var_then_body = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_then_body, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    Value wyrm_var_elif_clauses = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("elif")))) {
    wyrm_fn_advance();
    Value wyrm_var_elif_cond = wyrm_fn_parse_expression();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    Value wyrm_var_elif_body = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_elif_body, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    val_array_append(wyrm_var_elif_clauses, val_array_init(2, wyrm_var_elif_cond, wyrm_var_elif_body));
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    Value wyrm_var_else_body = val_array_init(0);
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("else")))) {
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_else_body, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    }
    return val_array_init(5, val_string("If"), wyrm_var_cond, wyrm_var_then_body, wyrm_var_elif_clauses, wyrm_var_else_body);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("repeat")))) {
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    Value wyrm_var_body = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_body, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    wyrm_fn_expect(val_string("KEYWORD"), val_string("til"));
    Value wyrm_var_cond = wyrm_fn_parse_expression();
    return val_array_init(3, val_string("Repeat"), wyrm_var_body, wyrm_var_cond);
    }
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("try")), val_eq(wyrm_fn_peek_val(), val_string("{"))))) {
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    Value wyrm_var_try_body = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_try_body, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    wyrm_fn_expect(val_string("KEYWORD"), val_string("catch"));
    Value wyrm_var_err_var = wyrm_fn_cur_val();
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    Value wyrm_var_catch_body = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_catch_body, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    return val_array_init(4, val_string("TryCatch"), wyrm_var_try_body, wyrm_var_err_var, wyrm_var_catch_body);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("unsafe")))) {
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    Value wyrm_var_body = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else {
    val_array_append(wyrm_var_body, wyrm_fn_parse_statement());
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    return val_array_init(2, val_string("Unsafe"), wyrm_var_body);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("return")))) {
    wyrm_fn_advance();
    Value wyrm_var_expr = val_null();
    if (val_to_bool(val_and(val_and(val_ne(wyrm_fn_cur_type(), val_string("NEWLINE")), val_ne(wyrm_fn_cur_val(), val_string("}"))), val_ne(wyrm_fn_cur_val(), val_string(";"))))) {
    wyrm_var_expr = wyrm_fn_parse_expression();
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(";")))) {
    wyrm_fn_advance();
    }
    return val_array_init(2, val_string("Return"), wyrm_var_expr);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("break")))) {
    wyrm_fn_advance();
    return val_array_init(1, val_string("Break"));
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("continue")))) {
    wyrm_fn_advance();
    return val_array_init(1, val_string("Continue"));
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("print")))) {
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("("));
    Value wyrm_var_exprs = val_array_init(0);
    if (val_to_bool(val_ne(wyrm_fn_cur_val(), val_string(")")))) {
    do {
    val_array_append(wyrm_var_exprs, wyrm_fn_parse_expression());
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(",")))) {
    wyrm_fn_advance();
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    }
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    return val_array_init(2, val_string("Print"), wyrm_var_exprs);
    }
    }
    Value wyrm_var_expr = wyrm_fn_parse_expression();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("=")))) {
    wyrm_fn_advance();
    Value wyrm_var_val = wyrm_fn_parse_expression();
    if (val_to_bool(val_eq(val_array_get(wyrm_var_expr, val_number(0)), val_string("Index")))) {
    return val_array_init(4, val_string("IndexAssign"), val_array_get(wyrm_var_expr, val_number(1)), val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_val);
    }
    return val_array_init(3, val_string("Assign"), val_array_get(wyrm_var_expr, val_number(1)), wyrm_var_val);
    }
    return wyrm_var_expr;
    return val_null();
}

Value wyrm_fn_parse_expression() {
    return wyrm_fn_parse_catch();
    return val_null();
}

Value wyrm_fn_parse_catch() {
    Value wyrm_var_node = wyrm_fn_parse_logical_or();
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("catch"))))) {
    wyrm_fn_advance();
    Value wyrm_var_right = wyrm_fn_parse_logical_or();
    wyrm_var_node = val_array_init(3, val_string("CatchExpr"), wyrm_var_node, wyrm_var_right);
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_node;
    return val_null();
}

Value wyrm_fn_parse_logical_or() {
    Value wyrm_var_node = wyrm_fn_parse_logical_and();
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")), val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("or")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("||")))))) {
    wyrm_fn_advance();
    Value wyrm_var_right = wyrm_fn_parse_logical_and();
    wyrm_var_node = val_array_init(4, val_string("BinaryOp"), val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_node, wyrm_var_right);
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_node;
    return val_null();
}

Value wyrm_fn_parse_logical_and() {
    Value wyrm_var_node = wyrm_fn_parse_equality();
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")), val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("and")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("&&")))))) {
    wyrm_fn_advance();
    Value wyrm_var_right = wyrm_fn_parse_equality();
    wyrm_var_node = val_array_init(4, val_string("BinaryOp"), val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_node, wyrm_var_right);
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_node;
    return val_null();
}

Value wyrm_fn_parse_equality() {
    Value wyrm_var_node = wyrm_fn_parse_comparison();
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("OPERATOR")), val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("==")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("!=")))))) {
    wyrm_fn_advance();
    Value wyrm_var_right = wyrm_fn_parse_comparison();
    wyrm_var_node = val_array_init(4, val_string("BinaryOp"), val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_node, wyrm_var_right);
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_node;
    return val_null();
}

Value wyrm_fn_parse_comparison() {
    Value wyrm_var_node = wyrm_fn_parse_addition_subtraction();
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("OPERATOR")), val_or(val_or(val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("<")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string(">"))), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("<="))), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string(">=")))))) {
    wyrm_fn_advance();
    Value wyrm_var_right = wyrm_fn_parse_addition_subtraction();
    wyrm_var_node = val_array_init(4, val_string("BinaryOp"), val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_node, wyrm_var_right);
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_node;
    return val_null();
}

Value wyrm_fn_parse_addition_subtraction() {
    Value wyrm_var_node = wyrm_fn_parse_multiplication_division();
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("OPERATOR")), val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("+")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("-")))))) {
    wyrm_fn_advance();
    Value wyrm_var_right = wyrm_fn_parse_multiplication_division();
    wyrm_var_node = val_array_init(4, val_string("BinaryOp"), val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_node, wyrm_var_right);
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_node;
    return val_null();
}

Value wyrm_fn_parse_multiplication_division() {
    Value wyrm_var_node = wyrm_fn_parse_power();
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("OPERATOR")), val_or(val_or(val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("*")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("/"))), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("//"))), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("%")))))) {
    wyrm_fn_advance();
    Value wyrm_var_right = wyrm_fn_parse_power();
    wyrm_var_node = val_array_init(4, val_string("BinaryOp"), val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_node, wyrm_var_right);
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_node;
    return val_null();
}

Value wyrm_fn_parse_power() {
    Value wyrm_var_node = wyrm_fn_parse_unary();
    do {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("OPERATOR")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("**"))))) {
    wyrm_fn_advance();
    Value wyrm_var_right = wyrm_fn_parse_unary();
    wyrm_var_node = val_array_init(4, val_string("BinaryOp"), val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_node, wyrm_var_right);
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    return wyrm_var_node;
    return val_null();
}

Value wyrm_fn_parse_unary() {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_or(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("OPERATOR")), val_or(val_or(val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("not")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("!"))), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("-"))), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("+")))), val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("not")))))) {
    wyrm_fn_advance();
    Value wyrm_var_expr = wyrm_fn_parse_unary();
    return val_array_init(3, val_string("UnaryOp"), val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_expr);
    }
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("try"))))) {
    wyrm_fn_advance();
    Value wyrm_var_expr = wyrm_fn_parse_unary();
    return val_array_init(2, val_string("TryExpr"), wyrm_var_expr);
    }
    return wyrm_fn_parse_primary();
    return val_null();
}

Value wyrm_fn_parse_primary() {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("NUMBER")))) {
    wyrm_fn_advance();
    return val_array_init(2, val_string("Number"), val_array_get(wyrm_var_tok, val_number(1)));
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("STRING")))) {
    wyrm_fn_advance();
    return val_array_init(2, val_string("String"), val_array_get(wyrm_var_tok, val_number(1)));
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")))) {
    if (val_to_bool(val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("true")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("false"))))) {
    wyrm_fn_advance();
    return val_array_init(2, val_string("Boolean"), val_array_get(wyrm_var_tok, val_number(1)));
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("null")))) {
    wyrm_fn_advance();
    return val_array_init(1, val_string("None"));
    }
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("IDENTIFIER")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_tok, val_number(1));
    wyrm_fn_advance();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("(")))) {
    wyrm_fn_advance();
    Value wyrm_var_args = val_array_init(0);
    if (val_to_bool(val_ne(wyrm_fn_cur_val(), val_string(")")))) {
    do {
    val_array_append(wyrm_var_args, wyrm_fn_parse_expression());
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(",")))) {
    wyrm_fn_advance();
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    }
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    return val_array_init(3, val_string("FuncCall"), val_array_init(2, val_string("Identifier"), wyrm_var_name), wyrm_var_args);
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("[")))) {
    wyrm_fn_advance();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(":")))) {
    wyrm_fn_advance();
    Value wyrm_var_end = wyrm_fn_parse_expression();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("]"));
    return val_array_init(3, val_string("Index"), val_array_init(2, val_string("Identifier"), wyrm_var_name), val_array_init(3, val_string("Slice"), val_null(), wyrm_var_end));
    }
    Value wyrm_var_expr = wyrm_fn_parse_expression();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(":")))) {
    wyrm_fn_advance();
    Value wyrm_var_end = val_null();
    if (val_to_bool(val_ne(wyrm_fn_cur_val(), val_string("]")))) {
    wyrm_var_end = wyrm_fn_parse_expression();
    }
    wyrm_fn_expect(val_string("DELIMITER"), val_string("]"));
    return val_array_init(3, val_string("Index"), val_array_init(2, val_string("Identifier"), wyrm_var_name), val_array_init(3, val_string("Slice"), wyrm_var_expr, wyrm_var_end));
    }
    wyrm_fn_expect(val_string("DELIMITER"), val_string("]"));
    return val_array_init(3, val_string("Index"), val_array_init(2, val_string("Identifier"), wyrm_var_name), wyrm_var_expr);
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(".")))) {
    wyrm_fn_advance();
    Value wyrm_var_method = wyrm_fn_cur_val();
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("("));
    if (val_to_bool(val_eq(wyrm_var_method, val_string("alloc")))) {
    Value wyrm_var_size = wyrm_fn_parse_expression();
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    return val_array_init(3, val_string("ArenaAlloc"), wyrm_var_name, wyrm_var_size);
    }
    if (val_to_bool(val_eq(wyrm_var_method, val_string("reset")))) {
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    return val_array_init(2, val_string("ArenaReset"), wyrm_var_name);
    }
    val_print(1, val_add(val_add(val_string("Parser Error: Unknown arena method '"), wyrm_var_method), val_string("'")));
    val_exit(val_number(1));
    }
    return val_array_init(2, val_string("Identifier"), wyrm_var_name);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("(")))) {
    wyrm_fn_advance();
    Value wyrm_var_expr = wyrm_fn_parse_expression();
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    return wyrm_var_expr;
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("[")))) {
    wyrm_fn_advance();
    Value wyrm_var_elements = val_array_init(0);
    if (val_to_bool(val_ne(wyrm_fn_cur_val(), val_string("]")))) {
    do {
    val_array_append(wyrm_var_elements, wyrm_fn_parse_expression());
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(",")))) {
    wyrm_fn_advance();
    }
    else {
    break;
    }
    } while (!val_to_bool(val_bool(false)));
    }
    wyrm_fn_expect(val_string("DELIMITER"), val_string("]"));
    return val_array_init(2, val_string("List"), wyrm_var_elements);
    }
    val_print(6, val_string("Parser Error: Unexpected token"), val_array_get(wyrm_var_tok, val_number(0)), val_add(val_add(val_string("'"), val_array_get(wyrm_var_tok, val_number(1))), val_string("' at line")), val_array_get(wyrm_var_tok, val_number(2)), val_string("col"), val_array_get(wyrm_var_tok, val_number(3)));
    val_exit(val_number(1));
    return val_null();
}

Value wyrm_fn_gen_temp() {
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    Value wyrm_var_name = val_add(val_string("%tmp_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    if (val_to_bool(val_array_get(wyrm_var_in_function, val_number(0)))) {
    val_array_append(val_array_get(wyrm_var_g_allocas, val_number(0)), val_add(val_add(val_string("  "), wyrm_var_name), val_string(" = alloca %struct.Value, align 8")));
    }
    else {
    val_array_append(val_array_get(wyrm_var_g_main_allocas, val_number(0)), val_add(val_add(val_string("  "), wyrm_var_name), val_string(" = alloca %struct.Value, align 8")));
    }
    return wyrm_var_name;
    return val_null();
}

Value wyrm_fn_emit_inst(Value wyrm_var_inst) {
    if (val_to_bool(val_array_get(wyrm_var_in_function, val_number(0)))) {
    val_array_append(val_array_get(wyrm_var_g_insts, val_number(0)), val_add(val_string("  "), wyrm_var_inst));
    }
    else {
    val_array_append(wyrm_var_g_llvm_main, val_add(val_string("  "), wyrm_var_inst));
    }
    return val_null();
}

Value wyrm_fn_llvm_escape_string(Value wyrm_var_s) {
    Value wyrm_var_res = val_string("");
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_l = val_len(wyrm_var_s);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    Value wyrm_var_c = val_char_at(wyrm_var_s, wyrm_var_i);
    if (val_to_bool(val_eq(wyrm_var_c, val_string("\n")))) {
    wyrm_var_res = val_add(wyrm_var_res, val_string("\\0A"));
    }
    else if (val_to_bool(val_eq(wyrm_var_c, val_string("\t")))) {
    wyrm_var_res = val_add(wyrm_var_res, val_string("\\09"));
    }
    else if (val_to_bool(val_eq(wyrm_var_c, val_string("\r")))) {
    wyrm_var_res = val_add(wyrm_var_res, val_string("\\0D"));
    }
    else if (val_to_bool(val_eq(wyrm_var_c, val_string("\\")))) {
    wyrm_var_res = val_add(wyrm_var_res, val_string("\\5C"));
    }
    else if (val_to_bool(val_eq(wyrm_var_c, val_string("\"")))) {
    wyrm_var_res = val_add(wyrm_var_res, val_string("\\22"));
    }
    else {
    wyrm_var_res = val_add(wyrm_var_res, wyrm_var_c);
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    return wyrm_var_res;
    return val_null();
}

Value wyrm_fn_push_scope() {
    val_array_append(val_array_get(wyrm_var_g_scopes, val_number(0)), val_array_init(2, val_array_init(0), val_array_init(0)));
    return val_null();
}

Value wyrm_fn_pop_scope() {
    Value wyrm_var_scopes_list = val_array_get(wyrm_var_g_scopes, val_number(0));
    Value wyrm_var_scope_idx = val_sub(val_len(wyrm_var_scopes_list), val_number(1));
    Value wyrm_var_scope = val_array_get(wyrm_var_scopes_list, wyrm_var_scope_idx);
    Value wyrm_var_arenas = val_array_get(wyrm_var_scope, val_number(0));
    Value wyrm_var_mallocs = val_array_get(wyrm_var_scope, val_number(1));
    Value wyrm_var_j = val_number(0);
    Value wyrm_var_ml = val_len(wyrm_var_mallocs);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_ml))) {
    break;
    }
    Value wyrm_var_m = val_array_get(wyrm_var_mallocs, wyrm_var_j);
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_raw_free(ptr "), wyrm_var_t), val_string(", ptr %wyrm_var_")), wyrm_var_m), val_string(")")));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_ml)));
    Value wyrm_var_k = val_number(0);
    Value wyrm_var_al = val_len(wyrm_var_arenas);
    do {
    if (val_to_bool(val_ge(wyrm_var_k, wyrm_var_al))) {
    break;
    }
    Value wyrm_var_a = val_array_get(wyrm_var_arenas, wyrm_var_k);
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @arena_destroy(ptr %wyrm_arena_"), wyrm_var_a), val_string(")")));
    wyrm_var_k = val_add(wyrm_var_k, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_k, wyrm_var_al)));
    val_array_pop(val_array_get(wyrm_var_g_scopes, val_number(0)));
    return val_null();
}

Value wyrm_fn_gen_all_cleanups() {
    Value wyrm_var_scopes_list = val_array_get(wyrm_var_g_scopes, val_number(0));
    Value wyrm_var_i = val_sub(val_len(wyrm_var_scopes_list), val_number(1));
    do {
    if (val_to_bool(val_lt(wyrm_var_i, val_number(0)))) {
    break;
    }
    Value wyrm_var_scope = val_array_get(wyrm_var_scopes_list, wyrm_var_i);
    Value wyrm_var_arenas = val_array_get(wyrm_var_scope, val_number(0));
    Value wyrm_var_mallocs = val_array_get(wyrm_var_scope, val_number(1));
    Value wyrm_var_j = val_number(0);
    Value wyrm_var_ml = val_len(wyrm_var_mallocs);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_ml))) {
    break;
    }
    Value wyrm_var_m = val_array_get(wyrm_var_mallocs, wyrm_var_j);
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_raw_free(ptr "), wyrm_var_t), val_string(", ptr %wyrm_var_")), wyrm_var_m), val_string(")")));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_ml)));
    Value wyrm_var_k = val_number(0);
    Value wyrm_var_al = val_len(wyrm_var_arenas);
    do {
    if (val_to_bool(val_ge(wyrm_var_k, wyrm_var_al))) {
    break;
    }
    Value wyrm_var_a = val_array_get(wyrm_var_arenas, wyrm_var_k);
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @arena_destroy(ptr %wyrm_arena_"), wyrm_var_a), val_string(")")));
    wyrm_var_k = val_add(wyrm_var_k, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_k, wyrm_var_al)));
    wyrm_var_i = val_sub(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_lt(wyrm_var_i, val_number(0))));
    return val_null();
}

Value wyrm_fn_has_dot(Value wyrm_var_s) {
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_l = val_len(wyrm_var_s);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    if (val_to_bool(val_eq(val_char_at(wyrm_var_s, wyrm_var_i), val_string(".")))) {
    return val_bool(true);
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    return val_bool(false);
    return val_null();
}

Value wyrm_fn_gen_expr(Value wyrm_var_node) {
    if (val_to_bool(val_eq(wyrm_var_node, val_null()))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_null(ptr "), wyrm_var_t), val_string(")")));
    return wyrm_var_t;
    }
    Value wyrm_var_type = val_array_get(wyrm_var_node, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Number")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_val = val_array_get(wyrm_var_node, val_number(1));
    if (val_to_bool(val_not(wyrm_fn_has_dot(wyrm_var_val)))) {
    wyrm_var_val = val_add(wyrm_var_val, val_string(".0"));
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_number(ptr "), wyrm_var_t), val_string(", double ")), wyrm_var_val), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("String")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_str_idx = val_len(wyrm_var_g_llvm_globals);
    Value wyrm_var_str_name = val_add(val_string("@.str."), val_str(wyrm_var_str_idx));
    Value wyrm_var_escaped = wyrm_fn_llvm_escape_string(val_array_get(wyrm_var_node, val_number(1)));
    Value wyrm_var_str_len = val_add(val_len(val_array_get(wyrm_var_node, val_number(1))), val_number(1));
    val_array_append(wyrm_var_g_llvm_globals, val_add(val_add(val_add(val_add(val_add(wyrm_var_str_name, val_string(" = private unnamed_addr constant [")), val_str(wyrm_var_str_len)), val_string(" x i8] c\"")), wyrm_var_escaped), val_string("\\00\", align 1")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_string(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_str_name), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Boolean")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_val_bit = val_string("0");
    if (val_to_bool(val_eq(val_array_get(wyrm_var_node, val_number(1)), val_string("true")))) {
    wyrm_var_val_bit = val_string("1");
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_bool(ptr "), wyrm_var_t), val_string(", i1 ")), wyrm_var_val_bit), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("None")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_null(ptr "), wyrm_var_t), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("TryExpr")))) {
    Value wyrm_var_sub_expr = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_reg = wyrm_fn_gen_expr(wyrm_var_sub_expr);
    Value wyrm_var_type_ptr = val_add(val_string("%type_ptr_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_type_val = val_add(val_string("%type_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_is_err = val_add(val_string("%is_err_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_type_ptr, val_string(" = getelementptr %struct.Value, ptr ")), wyrm_var_reg), val_string(", i32 0, i32 0")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_type_val, val_string(" = load i32, ptr ")), wyrm_var_type_ptr), val_string(", align 4")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_is_err, val_string(" = icmp eq i32 ")), wyrm_var_type_val), val_string(", 6")));
    Value wyrm_var_label_idx = val_array_get(wyrm_var_temp_count, val_number(0));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    Value wyrm_var_err_branch = val_add(val_string("try_err_"), val_str(wyrm_var_label_idx));
    Value wyrm_var_ok_branch = val_add(val_string("try_ok_"), val_str(wyrm_var_label_idx));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_string("br i1 "), wyrm_var_is_err), val_string(", label %")), wyrm_var_err_branch), val_string(", label %")), wyrm_var_ok_branch));
    wyrm_fn_emit_label(val_add(wyrm_var_err_branch, val_string(":")));
    Value wyrm_var_active_labels = val_array_get(wyrm_var_g_catch_labels, val_number(0));
    Value wyrm_var_num_labels = val_len(wyrm_var_active_labels);
    if (val_to_bool(val_gt(wyrm_var_num_labels, val_number(0)))) {
    Value wyrm_var_active_var = val_array_get(wyrm_var_g_catch_vars, val_number(0));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr "), wyrm_var_active_var), val_string(", ptr ")), wyrm_var_reg), val_string(")")));
    Value wyrm_var_active_label = val_array_get(wyrm_var_active_labels, val_sub(wyrm_var_num_labels, val_number(1)));
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_active_label));
    }
    else {
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_copy(ptr %result_ptr, ptr "), wyrm_var_reg), val_string(")")));
    wyrm_fn_gen_all_cleanups();
    wyrm_fn_emit_inst(val_string("ret void"));
    }
    wyrm_fn_emit_label(val_add(wyrm_var_ok_branch, val_string(":")));
    return wyrm_var_reg;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("CatchExpr")))) {
    Value wyrm_var_sub_expr = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_fallback_expr = val_array_get(wyrm_var_node, val_number(2));
    Value wyrm_var_res_var = wyrm_fn_gen_temp();
    Value wyrm_var_reg = wyrm_fn_gen_expr(wyrm_var_sub_expr);
    Value wyrm_var_type_ptr = val_add(val_string("%type_ptr_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_type_val = val_add(val_string("%type_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_is_err = val_add(val_string("%is_err_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_type_ptr, val_string(" = getelementptr %struct.Value, ptr ")), wyrm_var_reg), val_string(", i32 0, i32 0")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_type_val, val_string(" = load i32, ptr ")), wyrm_var_type_ptr), val_string(", align 4")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_is_err, val_string(" = icmp eq i32 ")), wyrm_var_type_val), val_string(", 6")));
    Value wyrm_var_label_idx = val_array_get(wyrm_var_temp_count, val_number(0));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    Value wyrm_var_err_branch = val_add(val_string("catch_err_"), val_str(wyrm_var_label_idx));
    Value wyrm_var_ok_branch = val_add(val_string("catch_ok_"), val_str(wyrm_var_label_idx));
    Value wyrm_var_merge_branch = val_add(val_string("catch_merge_"), val_str(wyrm_var_label_idx));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_string("br i1 "), wyrm_var_is_err), val_string(", label %")), wyrm_var_err_branch), val_string(", label %")), wyrm_var_ok_branch));
    wyrm_fn_emit_label(val_add(wyrm_var_err_branch, val_string(":")));
    Value wyrm_var_fallback_reg = wyrm_fn_gen_expr(wyrm_var_fallback_expr);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr "), wyrm_var_res_var), val_string(", ptr ")), wyrm_var_fallback_reg), val_string(")")));
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_merge_branch));
    wyrm_fn_emit_label(val_add(wyrm_var_ok_branch, val_string(":")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr "), wyrm_var_res_var), val_string(", ptr ")), wyrm_var_reg), val_string(")")));
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_merge_branch));
    wyrm_fn_emit_label(val_add(wyrm_var_merge_branch, val_string(":")));
    return wyrm_var_res_var;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Identifier")))) {
    if (val_to_bool(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_globals, val_number(0)), val_array_get(wyrm_var_node, val_number(1))))) {
    return val_add(val_string("@wyrm_var_"), val_array_get(wyrm_var_node, val_number(1)));
    }
    return val_add(val_string("%wyrm_var_"), val_array_get(wyrm_var_node, val_number(1)));
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("BinaryOp")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_left_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(2)));
    Value wyrm_var_right_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(3)));
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_func = val_string("");
    if (val_to_bool(val_eq(wyrm_var_op, val_string("+")))) {
    wyrm_var_func = val_string("llvm_val_add");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("-")))) {
    wyrm_var_func = val_string("llvm_val_sub");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("*")))) {
    wyrm_var_func = val_string("llvm_val_mul");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("/")))) {
    wyrm_var_func = val_string("llvm_val_div");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("%")))) {
    wyrm_var_func = val_string("llvm_val_mod");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("//")))) {
    wyrm_var_func = val_string("llvm_val_floordiv");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("==")))) {
    wyrm_var_func = val_string("llvm_val_eq");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("!=")))) {
    wyrm_var_func = val_string("llvm_val_ne");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("<")))) {
    wyrm_var_func = val_string("llvm_val_lt");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string(">")))) {
    wyrm_var_func = val_string("llvm_val_gt");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("<=")))) {
    wyrm_var_func = val_string("llvm_val_le");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string(">=")))) {
    wyrm_var_func = val_string("llvm_val_ge");
    }
    else if (val_to_bool(val_or(val_eq(wyrm_var_op, val_string("and")), val_eq(wyrm_var_op, val_string("&&"))))) {
    wyrm_var_func = val_string("llvm_val_and");
    }
    else if (val_to_bool(val_or(val_eq(wyrm_var_op, val_string("or")), val_eq(wyrm_var_op, val_string("||"))))) {
    wyrm_var_func = val_string("llvm_val_or");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("**")))) {
    wyrm_var_func = val_string("llvm_val_pow");
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @"), wyrm_var_func), val_string("(ptr ")), wyrm_var_t), val_string(", ptr ")), wyrm_var_left_reg), val_string(", ptr ")), wyrm_var_right_reg), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("UnaryOp")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_expr_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(2)));
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    if (val_to_bool(val_eq(wyrm_var_op, val_string("-")))) {
    Value wyrm_var_zero_reg = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_number(ptr "), wyrm_var_zero_reg), val_string(", double 0.0)")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_sub(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_zero_reg), val_string(", ptr ")), wyrm_var_expr_reg), val_string(")")));
    }
    else if (val_to_bool(val_or(val_eq(wyrm_var_op, val_string("!")), val_eq(wyrm_var_op, val_string("not"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_not(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_expr_reg), val_string(")")));
    }
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("List")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_elems = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_num_elems = val_len(wyrm_var_elems);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_array_create(ptr "), wyrm_var_t), val_string(", i32 ")), val_str(wyrm_var_num_elems)), val_string(")")));
    Value wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_elems))) {
    break;
    }
    Value wyrm_var_elem_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_elems, wyrm_var_i));
    Value wyrm_var_idx_reg = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_number(ptr "), wyrm_var_idx_reg), val_string(", double ")), val_str(wyrm_var_i)), val_string(".0)")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_array_set(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_idx_reg), val_string(", ptr ")), wyrm_var_elem_reg), val_string(")")));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_elems)));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Index")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_obj_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(1)));
    Value wyrm_var_idx_node = val_array_get(wyrm_var_node, val_number(2));
    if (val_to_bool(val_eq(val_array_get(wyrm_var_idx_node, val_number(0)), val_string("Slice")))) {
    Value wyrm_var_start_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_idx_node, val_number(1)));
    Value wyrm_var_end_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_idx_node, val_number(2)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_array_slice(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_obj_reg), val_string(", ptr ")), wyrm_var_start_reg), val_string(", ptr ")), wyrm_var_end_reg), val_string(")")));
    }
    else {
    Value wyrm_var_idx_reg = wyrm_fn_gen_expr(wyrm_var_idx_node);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_array_get(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_obj_reg), val_string(", ptr ")), wyrm_var_idx_reg), val_string(")")));
    }
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("ArenaAlloc")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_size_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(2)));
    Value wyrm_var_arena_name = val_array_get(wyrm_var_node, val_number(1));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_raw_malloc(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_size_reg), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("ArenaReset")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_null(ptr "), wyrm_var_t), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("FuncCall")))) {
    Value wyrm_var_name_node = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_name = val_array_get(wyrm_var_name_node, val_number(1));
    Value wyrm_var_args = val_array_get(wyrm_var_node, val_number(2));
    Value wyrm_var_num_args = val_len(wyrm_var_args);
    Value wyrm_var_arg_regs = val_array_init(0);
    Value wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_args))) {
    break;
    }
    val_array_append(wyrm_var_arg_regs, wyrm_fn_gen_expr(val_array_get(wyrm_var_args, wyrm_var_i)));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_args)));
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_is_builtin = val_bool(false);
    Value wyrm_var_builtin_func = val_string("");
    if (val_to_bool(val_eq(wyrm_var_name, val_string("len")))) {
    wyrm_var_builtin_func = val_string("llvm_val_len");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("type")))) {
    wyrm_var_builtin_func = val_string("llvm_val_type");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("int")))) {
    wyrm_var_builtin_func = val_string("llvm_val_int");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("float")))) {
    wyrm_var_builtin_func = val_string("llvm_val_float");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("str")))) {
    wyrm_var_builtin_func = val_string("llvm_val_str");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("abs")))) {
    wyrm_var_builtin_func = val_string("llvm_val_abs");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("round")))) {
    wyrm_var_builtin_func = val_string("llvm_val_round");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("pow")))) {
    wyrm_var_builtin_func = val_string("llvm_val_pow");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("append")))) {
    wyrm_var_builtin_func = val_string("llvm_val_array_append");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("pop")))) {
    wyrm_var_builtin_func = val_string("llvm_val_array_pop");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("malloc")))) {
    wyrm_var_builtin_func = val_string("llvm_val_raw_malloc");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("free")))) {
    wyrm_var_builtin_func = val_string("llvm_val_raw_free");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("realloc")))) {
    wyrm_var_builtin_func = val_string("llvm_val_raw_realloc");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sys_args")))) {
    wyrm_var_builtin_func = val_string("val_sys_args");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("read_file")))) {
    wyrm_var_builtin_func = val_string("llvm_val_read_file");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("error")))) {
    wyrm_var_builtin_func = val_string("llvm_val_error_val");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("write_file")))) {
    wyrm_var_builtin_func = val_string("llvm_val_write_file");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("exit")))) {
    wyrm_var_builtin_func = val_string("llvm_val_exit");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("system")))) {
    wyrm_var_builtin_func = val_string("llvm_val_system");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("getenv")))) {
    wyrm_var_builtin_func = val_string("llvm_val_getenv");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("split")))) {
    wyrm_var_builtin_func = val_string("llvm_val_split");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("join")))) {
    wyrm_var_builtin_func = val_string("llvm_val_join");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("trim")))) {
    wyrm_var_builtin_func = val_string("llvm_val_trim");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("upper")))) {
    wyrm_var_builtin_func = val_string("llvm_val_upper");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("lower")))) {
    wyrm_var_builtin_func = val_string("llvm_val_lower");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("contains")))) {
    wyrm_var_builtin_func = val_string("llvm_val_contains");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("replace")))) {
    wyrm_var_builtin_func = val_string("llvm_val_replace");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("starts_with")))) {
    wyrm_var_builtin_func = val_string("llvm_val_starts_with");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("ends_with")))) {
    wyrm_var_builtin_func = val_string("llvm_val_ends_with");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("char_at")))) {
    wyrm_var_builtin_func = val_string("llvm_val_char_at");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("ord_val")))) {
    wyrm_var_builtin_func = val_string("llvm_val_ord_val");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("chr_val")))) {
    wyrm_var_builtin_func = val_string("llvm_val_chr_val");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("to_bytes")))) {
    wyrm_var_builtin_func = val_string("llvm_val_to_bytes");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("from_bytes")))) {
    wyrm_var_builtin_func = val_string("llvm_val_from_bytes");
    wyrm_var_is_builtin = val_bool(true);
    }
    if (val_to_bool(wyrm_var_is_builtin)) {
    if (val_to_bool(val_eq(wyrm_var_name, val_string("sys_args")))) {
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_copy(ptr "), wyrm_var_t), val_string(", ptr @wyrm_sys_args)")));
    }
    else {
    Value wyrm_var_call_args = val_add(val_string("ptr "), wyrm_var_t);
    Value wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_num_args))) {
    break;
    }
    wyrm_var_call_args = val_add(val_add(wyrm_var_call_args, val_string(", ptr ")), val_array_get(wyrm_var_arg_regs, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_num_args)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @"), wyrm_var_builtin_func), val_string("(")), wyrm_var_call_args), val_string(")")));
    }
    }
    else {
    Value wyrm_var_call_args = val_add(val_string("ptr "), wyrm_var_t);
    Value wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_num_args))) {
    break;
    }
    wyrm_var_call_args = val_add(val_add(wyrm_var_call_args, val_string(", ptr ")), val_array_get(wyrm_var_arg_regs, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_num_args)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @wyrm_fn_"), wyrm_var_name), val_string("(")), wyrm_var_call_args), val_string(")")));
    }
    return wyrm_var_t;
    }
    val_print(1, val_add(val_add(val_string("Code Generator Error: Unknown expression type '"), wyrm_var_type), val_string("'")));
    val_exit(val_number(1));
    return val_null();
}

Value wyrm_fn_gen_statement(Value wyrm_var_stmt) {
    Value wyrm_var_type = val_array_get(wyrm_var_stmt, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Use")))) {
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("FuncDef")))) {
    val_array_set(wyrm_var_in_function, val_number(0), val_bool(true));
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_params = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_body = val_array_get(wyrm_var_stmt, val_number(3));
    val_array_set(wyrm_var_g_allocas, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_g_insts, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_temp_count, val_number(0), val_number(0));
    val_array_set(wyrm_var_g_scopes, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_declared_locals, val_number(0), val_array_init(0));
    wyrm_fn_push_scope();
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_num_params = val_len(wyrm_var_params);
    Value wyrm_var_params_decl = val_string("ptr %result_ptr");
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_params))) {
    break;
    }
    wyrm_var_params_decl = val_add(val_add(wyrm_var_params_decl, val_string(", ptr %arg_")), val_array_get(wyrm_var_params, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_params)));
    wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_params))) {
    break;
    }
    Value wyrm_var_p_name = val_array_get(wyrm_var_params, wyrm_var_i);
    val_array_append(val_array_get(wyrm_var_g_allocas, val_number(0)), val_add(val_add(val_string("  %wyrm_var_"), wyrm_var_p_name), val_string(" = alloca %struct.Value, align 8")));
    val_array_append(val_array_get(wyrm_var_declared_locals, val_number(0)), wyrm_var_p_name);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr %wyrm_var_"), wyrm_var_p_name), val_string(", ptr %arg_")), wyrm_var_p_name), val_string(")")));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_params)));
    Value wyrm_var_j = val_number(0);
    Value wyrm_var_body_len = val_len(wyrm_var_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_body_len))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_body_len)));
    wyrm_fn_pop_scope();
    Value wyrm_var_func_code = val_add(val_add(val_add(val_add(val_string("define void @wyrm_fn_"), wyrm_var_name), val_string("(")), wyrm_var_params_decl), val_string(") {\n"));
    Value wyrm_var_k = val_number(0);
    Value wyrm_var_allocas_list = val_array_get(wyrm_var_g_allocas, val_number(0));
    Value wyrm_var_al = val_len(wyrm_var_allocas_list);
    do {
    if (val_to_bool(val_ge(wyrm_var_k, wyrm_var_al))) {
    break;
    }
    wyrm_var_func_code = val_add(val_add(wyrm_var_func_code, val_array_get(wyrm_var_allocas_list, wyrm_var_k)), val_string("\n"));
    wyrm_var_k = val_add(wyrm_var_k, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_k, wyrm_var_al)));
    wyrm_var_k = val_number(0);
    Value wyrm_var_insts_list = val_array_get(wyrm_var_g_insts, val_number(0));
    Value wyrm_var_il = val_len(wyrm_var_insts_list);
    do {
    if (val_to_bool(val_ge(wyrm_var_k, wyrm_var_il))) {
    break;
    }
    wyrm_var_func_code = val_add(val_add(wyrm_var_func_code, val_array_get(wyrm_var_insts_list, wyrm_var_k)), val_string("\n"));
    wyrm_var_k = val_add(wyrm_var_k, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_k, wyrm_var_il)));
    wyrm_var_func_code = val_add(wyrm_var_func_code, val_string("  call void @llvm_val_null(ptr %result_ptr)\n"));
    wyrm_var_func_code = val_add(wyrm_var_func_code, val_string("  ret void\n"));
    wyrm_var_func_code = val_add(wyrm_var_func_code, val_string("}\n"));
    val_array_append(wyrm_var_g_llvm_funcs, wyrm_var_func_code);
    val_array_set(wyrm_var_in_function, val_number(0), val_bool(false));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("VarDecl")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_val = val_array_get(wyrm_var_stmt, val_number(3));
    if (val_to_bool(val_array_get(wyrm_var_in_function, val_number(0)))) {
    if (val_to_bool(val_not(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_locals, val_number(0)), wyrm_var_name)))) {
    val_array_append(val_array_get(wyrm_var_g_allocas, val_number(0)), val_add(val_add(val_string("  %wyrm_var_"), wyrm_var_name), val_string(" = alloca %struct.Value, align 8")));
    val_array_append(val_array_get(wyrm_var_declared_locals, val_number(0)), wyrm_var_name);
    }
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_val);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr %wyrm_var_"), wyrm_var_name), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
    if (val_to_bool(val_and(val_and(val_array_get(wyrm_var_in_unsafe, val_number(0)), val_ne(wyrm_var_val, val_null())), val_eq(val_array_get(wyrm_var_val, val_number(0)), val_string("FuncCall"))))) {
    Value wyrm_var_call_func = val_array_get(wyrm_var_val, val_number(1));
    Value wyrm_var_call_name = val_array_get(wyrm_var_call_func, val_number(1));
    if (val_to_bool(val_eq(wyrm_var_call_name, val_string("malloc")))) {
    Value wyrm_var_scopes_list = val_array_get(wyrm_var_g_scopes, val_number(0));
    Value wyrm_var_scope_idx = val_sub(val_len(wyrm_var_scopes_list), val_number(1));
    Value wyrm_var_scope = val_array_get(wyrm_var_scopes_list, wyrm_var_scope_idx);
    Value wyrm_var_mallocs = val_array_get(wyrm_var_scope, val_number(1));
    val_array_append(wyrm_var_mallocs, wyrm_var_name);
    }
    }
    }
    else {
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_val);
    val_array_append(wyrm_var_g_llvm_globals, val_add(val_add(val_string("@wyrm_var_"), wyrm_var_name), val_string(" = global %struct.Value zeroinitializer, align 8")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr @wyrm_var_"), wyrm_var_name), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
    }
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Assign")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_val = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_val);
    if (val_to_bool(val_array_get(wyrm_var_in_function, val_number(0)))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr %wyrm_var_"), wyrm_var_name), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
    }
    else {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr @wyrm_var_"), wyrm_var_name), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
    }
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("IndexAssign")))) {
    Value wyrm_var_obj_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_stmt, val_number(1)));
    Value wyrm_var_idx_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_stmt, val_number(2)));
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_stmt, val_number(3)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_array_set(ptr "), wyrm_var_obj_reg), val_string(", ptr ")), wyrm_var_idx_reg), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Print")))) {
    Value wyrm_var_exprs = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_num_exprs = val_len(wyrm_var_exprs);
    Value wyrm_var_call_args = val_add(val_string("i32 "), val_str(wyrm_var_num_exprs));
    Value wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_exprs))) {
    break;
    }
    Value wyrm_var_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_exprs, wyrm_var_i));
    wyrm_var_call_args = val_add(val_add(wyrm_var_call_args, val_string(", ptr ")), wyrm_var_reg);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_exprs)));
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void (i32, ...) @llvm_val_print("), wyrm_var_call_args), val_string(")")));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("If")))) {
    Value wyrm_var_cond = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_then_body = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_elif_clauses = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_else_body = val_array_get(wyrm_var_stmt, val_number(4));
    Value wyrm_var_cond_reg = wyrm_fn_gen_expr(wyrm_var_cond);
    Value wyrm_var_cond_i1 = val_add(val_string("%cond_i1_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_cond_i1, val_string(" = call i1 @llvm_val_to_bool(ptr ")), wyrm_var_cond_reg), val_string(")")));
    Value wyrm_var_label_idx = val_array_get(wyrm_var_temp_count, val_number(0));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    Value wyrm_var_then_label = val_add(val_string("then_"), val_str(wyrm_var_label_idx));
    Value wyrm_var_next_label = val_add(val_string("next_"), val_str(wyrm_var_label_idx));
    Value wyrm_var_merge_label = val_add(val_string("merge_"), val_str(wyrm_var_label_idx));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_string("br i1 "), wyrm_var_cond_i1), val_string(", label %")), wyrm_var_then_label), val_string(", label %")), wyrm_var_next_label));
    wyrm_fn_emit_label(val_add(wyrm_var_then_label, val_string(":")));
    wyrm_fn_push_scope();
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_then_len = val_len(wyrm_var_then_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_then_len))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_then_body, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_then_len)));
    wyrm_fn_pop_scope();
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_merge_label));
    wyrm_fn_emit_label(val_add(wyrm_var_next_label, val_string(":")));
    if (val_to_bool(val_gt(val_len(wyrm_var_else_body), val_number(0)))) {
    wyrm_fn_push_scope();
    wyrm_var_i = val_number(0);
    Value wyrm_var_else_len = val_len(wyrm_var_else_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_else_len))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_else_body, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_else_len)));
    wyrm_fn_pop_scope();
    }
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_merge_label));
    wyrm_fn_emit_label(val_add(wyrm_var_merge_label, val_string(":")));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Repeat")))) {
    Value wyrm_var_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_cond = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_loop_label = val_add(val_string("loop_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_cond_label = val_add(val_string("cond_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_merge_label = val_add(val_string("merge_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_loop_label));
    wyrm_fn_emit_label(val_add(wyrm_var_loop_label, val_string(":")));
    wyrm_fn_push_scope();
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_body_len = val_len(wyrm_var_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_body_len))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_body, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_body_len)));
    wyrm_fn_pop_scope();
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_cond_label));
    wyrm_fn_emit_label(val_add(wyrm_var_cond_label, val_string(":")));
    Value wyrm_var_cond_reg = wyrm_fn_gen_expr(wyrm_var_cond);
    Value wyrm_var_cond_i1 = val_add(val_string("%cond_i1_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_cond_i1, val_string(" = call i1 @llvm_val_to_bool(ptr ")), wyrm_var_cond_reg), val_string(")")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_string("br i1 "), wyrm_var_cond_i1), val_string(", label %")), wyrm_var_merge_label), val_string(", label %")), wyrm_var_loop_label));
    wyrm_fn_emit_label(val_add(wyrm_var_merge_label, val_string(":")));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("TryCatch")))) {
    Value wyrm_var_try_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_err_var = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_catch_body = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_catch_label = val_add(val_string("catch_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_end_label = val_add(val_string("try_end_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    val_array_append(val_array_get(wyrm_var_g_catch_labels, val_number(0)), wyrm_var_catch_label);
    val_array_append(val_array_get(wyrm_var_g_catch_vars, val_number(0)), val_add(val_string("%wyrm_var_"), wyrm_var_err_var));
    val_array_append(val_array_get(wyrm_var_g_allocas, val_number(0)), val_add(val_add(val_string("  %wyrm_var_"), wyrm_var_err_var), val_string(" = alloca %struct.Value, align 8")));
    wyrm_fn_push_scope();
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_try_len = val_len(wyrm_var_try_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_try_len))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_try_body, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_try_len)));
    wyrm_fn_pop_scope();
    val_array_pop(val_array_get(wyrm_var_g_catch_labels, val_number(0)));
    val_array_pop(val_array_get(wyrm_var_g_catch_vars, val_number(0)));
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_end_label));
    wyrm_fn_emit_label(val_add(wyrm_var_catch_label, val_string(":")));
    wyrm_fn_push_scope();
    wyrm_var_i = val_number(0);
    Value wyrm_var_catch_len = val_len(wyrm_var_catch_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_catch_len))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_catch_body, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_catch_len)));
    wyrm_fn_pop_scope();
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_end_label));
    wyrm_fn_emit_label(val_add(wyrm_var_end_label, val_string(":")));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Unsafe")))) {
    Value wyrm_var_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_old_unsafe = val_array_get(wyrm_var_in_unsafe, val_number(0));
    val_array_set(wyrm_var_in_unsafe, val_number(0), val_bool(true));
    wyrm_fn_push_scope();
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_body_len = val_len(wyrm_var_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_body_len))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_body, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_body_len)));
    wyrm_fn_pop_scope();
    val_array_set(wyrm_var_in_unsafe, val_number(0), wyrm_var_old_unsafe);
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Arena")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_size = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_size_reg = wyrm_fn_gen_expr(wyrm_var_size);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("%wyrm_arena_"), wyrm_var_name), val_string(" = call ptr @val_arena_create_wrapper(ptr ")), wyrm_var_size_reg), val_string(")")));
    Value wyrm_var_scopes_list = val_array_get(wyrm_var_g_scopes, val_number(0));
    Value wyrm_var_scope_idx = val_sub(val_len(wyrm_var_scopes_list), val_number(1));
    Value wyrm_var_scope = val_array_get(wyrm_var_scopes_list, wyrm_var_scope_idx);
    Value wyrm_var_arenas = val_array_get(wyrm_var_scope, val_number(0));
    val_array_append(wyrm_var_arenas, wyrm_var_name);
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Return")))) {
    Value wyrm_var_expr = val_array_get(wyrm_var_stmt, val_number(1));
    if (val_to_bool(val_ne(wyrm_var_expr, val_null()))) {
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_expr);
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_copy(ptr %result_ptr, ptr "), wyrm_var_val_reg), val_string(")")));
    }
    else {
    wyrm_fn_emit_inst(val_string("call void @llvm_val_null(ptr %result_ptr)"));
    }
    wyrm_fn_gen_all_cleanups();
    wyrm_fn_emit_inst(val_string("ret void"));
    return val_null();
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_type, val_string("Break")), val_eq(wyrm_var_type, val_string("Continue"))))) {
    wyrm_fn_gen_all_cleanups();
    return val_null();
    }
    wyrm_fn_gen_expr(wyrm_var_stmt);
    return val_null();
}

Value wyrm_fn_emit_label(Value wyrm_var_label) {
    if (val_to_bool(val_array_get(wyrm_var_in_function, val_number(0)))) {
    val_array_append(val_array_get(wyrm_var_g_insts, val_number(0)), wyrm_var_label);
    }
    else {
    val_array_append(wyrm_var_g_llvm_main, wyrm_var_label);
    }
    return val_null();
}

Value wyrm_fn_check_expr(Value wyrm_var_expr, Value wyrm_var_error_funcs, Value wyrm_var_in_try_stmt, Value wyrm_var_in_try_expr, Value wyrm_var_in_catch_expr, Value wyrm_var_func_returns_error) {
    if (val_to_bool(val_eq(wyrm_var_expr, val_null()))) {
    return val_null();
    }
    if (val_to_bool(val_ne(val_type(wyrm_var_expr), val_string("Array")))) {
    return val_null();
    }
    Value wyrm_var_n_type = val_array_get(wyrm_var_expr, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("FuncCall")))) {
    Value wyrm_var_func_node = val_array_get(wyrm_var_expr, val_number(1));
    Value wyrm_var_name = val_array_get(wyrm_var_func_node, val_number(1));
    Value wyrm_var_is_err_func = val_bool(false);
    Value wyrm_var_j = val_number(0);
    Value wyrm_var_el = val_len(wyrm_var_error_funcs);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_el))) {
    break;
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_error_funcs, wyrm_var_j), wyrm_var_name))) {
    wyrm_var_is_err_func = val_bool(true);
    }
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_el)));
    if (val_to_bool(wyrm_var_is_err_func)) {
    if (val_to_bool(val_and(val_not(wyrm_var_in_try_expr), val_not(wyrm_var_in_catch_expr)))) {
    val_print(1, val_add(val_add(val_string("Compiler Error: Unhandled error union returned by call to '"), wyrm_var_name), val_string("'")));
    val_exit(val_number(1));
    }
    if (val_to_bool(wyrm_var_in_try_expr)) {
    if (val_to_bool(val_and(val_not(wyrm_var_in_try_stmt), val_not(wyrm_var_func_returns_error)))) {
    val_print(1, val_string("Compiler Error: Cannot propagate error via 'try' in a function that does not return an error union"));
    val_exit(val_number(1));
    }
    }
    }
    Value wyrm_var_args = val_array_get(wyrm_var_expr, val_number(2));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_al = val_len(wyrm_var_args);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_al))) {
    break;
    }
    wyrm_fn_check_expr(val_array_get(wyrm_var_args, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_al)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("TryExpr")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(true), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("CatchExpr")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(true), wyrm_var_func_returns_error);
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("BinaryOp")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(3)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("UnaryOp")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Array")))) {
    Value wyrm_var_elems = val_array_get(wyrm_var_expr, val_number(1));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_el = val_len(wyrm_var_elems);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_el))) {
    break;
    }
    wyrm_fn_check_expr(val_array_get(wyrm_var_elems, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_el)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Index")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Slice")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(3)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    return val_null();
}

Value wyrm_fn_check_stmt(Value wyrm_var_stmt, Value wyrm_var_error_funcs, Value wyrm_var_in_try_stmt, Value wyrm_var_func_returns_error) {
    if (val_to_bool(val_eq(wyrm_var_stmt, val_null()))) {
    return val_null();
    }
    if (val_to_bool(val_ne(val_type(wyrm_var_stmt), val_string("Array")))) {
    return val_null();
    }
    Value wyrm_var_n_type = val_array_get(wyrm_var_stmt, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("VarDecl")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_stmt, val_number(3)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Assign")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_stmt, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("IndexAssign")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_stmt, val_number(3)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Print")))) {
    Value wyrm_var_exprs = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_el = val_len(wyrm_var_exprs);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_el))) {
    break;
    }
    wyrm_fn_check_expr(val_array_get(wyrm_var_exprs, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_el)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Return")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_stmt, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Repeat")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_stmt, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    Value wyrm_var_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_bl = val_len(wyrm_var_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_bl))) {
    break;
    }
    wyrm_fn_check_stmt(val_array_get(wyrm_var_body, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_bl)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Unsafe")))) {
    Value wyrm_var_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_bl = val_len(wyrm_var_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_bl))) {
    break;
    }
    wyrm_fn_check_stmt(val_array_get(wyrm_var_body, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_bl)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("If")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_stmt, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    Value wyrm_var_then_body = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_bl = val_len(wyrm_var_then_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_bl))) {
    break;
    }
    wyrm_fn_check_stmt(val_array_get(wyrm_var_then_body, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_bl)));
    Value wyrm_var_elif_clauses = val_array_get(wyrm_var_stmt, val_number(3));
    wyrm_var_i = val_number(0);
    Value wyrm_var_ecl = val_len(wyrm_var_elif_clauses);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_ecl))) {
    break;
    }
    Value wyrm_var_clause = val_array_get(wyrm_var_elif_clauses, wyrm_var_i);
    wyrm_fn_check_expr(val_array_get(wyrm_var_clause, val_number(0)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    Value wyrm_var_elif_body = val_array_get(wyrm_var_clause, val_number(1));
    Value wyrm_var_j = val_number(0);
    Value wyrm_var_ebl = val_len(wyrm_var_elif_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_ebl))) {
    break;
    }
    wyrm_fn_check_stmt(val_array_get(wyrm_var_elif_body, wyrm_var_j), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_func_returns_error);
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_ebl)));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_ecl)));
    Value wyrm_var_else_body = val_array_get(wyrm_var_stmt, val_number(4));
    wyrm_var_i = val_number(0);
    Value wyrm_var_elbl = val_len(wyrm_var_else_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_elbl))) {
    break;
    }
    wyrm_fn_check_stmt(val_array_get(wyrm_var_else_body, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_elbl)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("TryCatch")))) {
    Value wyrm_var_try_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_catch_body = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_tbl = val_len(wyrm_var_try_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_tbl))) {
    break;
    }
    wyrm_fn_check_stmt(val_array_get(wyrm_var_try_body, wyrm_var_i), wyrm_var_error_funcs, val_bool(true), wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_tbl)));
    wyrm_var_i = val_number(0);
    Value wyrm_var_cbl = val_len(wyrm_var_catch_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_cbl))) {
    break;
    }
    wyrm_fn_check_stmt(val_array_get(wyrm_var_catch_body, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_cbl)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Arena")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_stmt, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(false), val_bool(false), wyrm_var_func_returns_error);
    }
    return val_null();
}

Value wyrm_fn_check_errors(Value wyrm_var_ast) {
    Value wyrm_var_error_funcs = val_array_init(1, val_string("read_file"));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_l = val_len(wyrm_var_ast);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    Value wyrm_var_stmt = val_array_get(wyrm_var_ast, wyrm_var_i);
    if (val_to_bool(val_eq(val_array_get(wyrm_var_stmt, val_number(0)), val_string("FuncDef")))) {
    Value wyrm_var_returns_error = val_array_get(wyrm_var_stmt, val_number(4));
    if (val_to_bool(wyrm_var_returns_error)) {
    val_array_append(wyrm_var_error_funcs, val_array_get(wyrm_var_stmt, val_number(1)));
    }
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    Value wyrm_var_stmt = val_array_get(wyrm_var_ast, wyrm_var_i);
    if (val_to_bool(val_eq(val_array_get(wyrm_var_stmt, val_number(0)), val_string("FuncDef")))) {
    Value wyrm_var_returns_error = val_array_get(wyrm_var_stmt, val_number(4));
    Value wyrm_var_body = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_j = val_number(0);
    Value wyrm_var_bl = val_len(wyrm_var_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_bl))) {
    break;
    }
    wyrm_fn_check_stmt(val_array_get(wyrm_var_body, wyrm_var_j), wyrm_var_error_funcs, val_bool(false), wyrm_var_returns_error);
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_bl)));
    }
    else {
    wyrm_fn_check_stmt(wyrm_var_stmt, wyrm_var_error_funcs, val_bool(false), val_bool(false));
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    return val_null();
}

Value wyrm_fn_transpile_llvm(Value wyrm_var_ast) {
    val_array_append(wyrm_var_g_llvm_globals, val_string("; Module generated by Wyrm 2.4 LLVM backend"));
    val_array_append(wyrm_var_g_llvm_globals, val_string("target datalayout = \"e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128\""));
    val_array_append(wyrm_var_g_llvm_globals, val_string("target triple = \"x86_64-pc-windows-msvc\""));
    val_array_append(wyrm_var_g_llvm_globals, val_string("%struct.Value = type { i32, %union.anon }"));
    val_array_append(wyrm_var_g_llvm_globals, val_string("%union.anon = type { double }"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_null(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_bool(ptr, i1)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_number(ptr, double)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_string(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_array_create(ptr, i32)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_array_init(ptr, i32, ...)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare i1 @llvm_val_to_bool(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_print(i32, ...)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_input(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_len(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_type(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_int(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_float(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_str(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_abs(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_round(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_pow(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_add(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sub(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_mul(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_div(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_mod(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_floordiv(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_eq(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_ne(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_lt(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_gt(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_le(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_ge(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_and(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_or(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_not(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_array_get(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_array_set(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_array_append(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_array_pop(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_array_slice(ptr, ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_raw_malloc(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_raw_realloc(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_raw_free(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_raw_ptr(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_arena_reset(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_read_file(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_error_val(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_write_file(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_exit(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_system(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_split(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_join(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_trim(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_upper(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_lower(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_contains(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_replace(ptr, ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_starts_with(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_ends_with(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_char_at(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_ord_val(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_chr_val(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_to_bytes(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_from_bytes(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_copy(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare ptr @val_arena_create_wrapper(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @arena_destroy(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @val_init_sys_args(i32, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("@wyrm_sys_args = external global %struct.Value, align 8"));
    val_array_set(wyrm_var_declared_globals, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_g_main_allocas, val_number(0), val_array_init(0));
    Value wyrm_var_g_idx = val_number(0);
    Value wyrm_var_g_len = val_len(wyrm_var_ast);
    do {
    if (val_to_bool(val_ge(wyrm_var_g_idx, wyrm_var_g_len))) {
    break;
    }
    Value wyrm_var_stmt = val_array_get(wyrm_var_ast, wyrm_var_g_idx);
    if (val_to_bool(val_eq(val_array_get(wyrm_var_stmt, val_number(0)), val_string("VarDecl")))) {
    val_array_append(val_array_get(wyrm_var_declared_globals, val_number(0)), val_array_get(wyrm_var_stmt, val_number(2)));
    }
    else if (val_to_bool(val_eq(val_array_get(wyrm_var_stmt, val_number(0)), val_string("Arena")))) {
    val_array_append(val_array_get(wyrm_var_declared_globals, val_number(0)), val_array_get(wyrm_var_stmt, val_number(1)));
    }
    wyrm_var_g_idx = val_add(wyrm_var_g_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_g_idx, wyrm_var_g_len)));
    Value wyrm_var_idx = val_number(0);
    Value wyrm_var_length = val_len(wyrm_var_ast);
    do {
    if (val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length))) {
    break;
    }
    Value wyrm_var_stmt = val_array_get(wyrm_var_ast, wyrm_var_idx);
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_stmt, val_number(0)), val_string("FuncDef")), val_eq(val_array_get(wyrm_var_stmt, val_number(1)), val_string("main"))))) {
    val_array_set(wyrm_var_has_main_def, val_number(0), val_bool(true));
    }
    wyrm_var_idx = val_add(wyrm_var_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length)));
    wyrm_var_idx = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_ast, wyrm_var_idx));
    wyrm_var_idx = val_add(wyrm_var_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length)));
    Value wyrm_var_result = val_string("");
    wyrm_var_idx = val_number(0);
    wyrm_var_length = val_len(wyrm_var_g_llvm_globals);
    do {
    if (val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length))) {
    break;
    }
    wyrm_var_result = val_add(val_add(wyrm_var_result, val_array_get(wyrm_var_g_llvm_globals, wyrm_var_idx)), val_string("\n"));
    wyrm_var_idx = val_add(wyrm_var_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length)));
    wyrm_var_result = val_add(wyrm_var_result, val_string("\n"));
    wyrm_var_idx = val_number(0);
    wyrm_var_length = val_len(wyrm_var_g_llvm_decls);
    do {
    if (val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length))) {
    break;
    }
    wyrm_var_result = val_add(val_add(wyrm_var_result, val_array_get(wyrm_var_g_llvm_decls, wyrm_var_idx)), val_string("\n"));
    wyrm_var_idx = val_add(wyrm_var_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length)));
    wyrm_var_result = val_add(wyrm_var_result, val_string("\n"));
    wyrm_var_idx = val_number(0);
    wyrm_var_length = val_len(wyrm_var_g_llvm_funcs);
    do {
    if (val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length))) {
    break;
    }
    wyrm_var_result = val_add(val_add(wyrm_var_result, val_array_get(wyrm_var_g_llvm_funcs, wyrm_var_idx)), val_string("\n"));
    wyrm_var_idx = val_add(wyrm_var_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length)));
    wyrm_var_result = val_add(wyrm_var_result, val_string("\n"));
    wyrm_var_result = val_add(wyrm_var_result, val_string("define i32 @main(i32 %argc, ptr %argv) {\n"));
    wyrm_var_result = val_add(wyrm_var_result, val_string("  call void @val_init_sys_args(i32 %argc, ptr %argv)\n"));
    wyrm_var_idx = val_number(0);
    Value wyrm_var_m_allocs = val_array_get(wyrm_var_g_main_allocas, val_number(0));
    Value wyrm_var_m_len = val_len(wyrm_var_m_allocs);
    do {
    if (val_to_bool(val_ge(wyrm_var_idx, wyrm_var_m_len))) {
    break;
    }
    wyrm_var_result = val_add(val_add(wyrm_var_result, val_array_get(wyrm_var_m_allocs, wyrm_var_idx)), val_string("\n"));
    wyrm_var_idx = val_add(wyrm_var_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_idx, wyrm_var_m_len)));
    wyrm_var_idx = val_number(0);
    wyrm_var_length = val_len(wyrm_var_g_llvm_main);
    do {
    if (val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length))) {
    break;
    }
    wyrm_var_result = val_add(val_add(wyrm_var_result, val_array_get(wyrm_var_g_llvm_main, wyrm_var_idx)), val_string("\n"));
    wyrm_var_idx = val_add(wyrm_var_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_idx, wyrm_var_length)));
    if (val_to_bool(val_array_get(wyrm_var_has_main_def, val_number(0)))) {
    Value wyrm_var_t = val_string("%tmp_main_res");
    wyrm_var_result = val_add(val_add(val_add(wyrm_var_result, val_string("  ")), wyrm_var_t), val_string(" = alloca %struct.Value, align 8\n"));
    wyrm_var_result = val_add(val_add(val_add(wyrm_var_result, val_string("  call void @wyrm_fn_main(ptr ")), wyrm_var_t), val_string(")\n"));
    }
    wyrm_var_result = val_add(wyrm_var_result, val_string("  ret i32 0\n"));
    wyrm_var_result = val_add(wyrm_var_result, val_string("}\n"));
    return wyrm_var_result;
    return val_null();
}

Value wyrm_fn_parse_file(Value wyrm_var_path, Value wyrm_var_processed_files) {
    if (val_to_bool(wyrm_fn_contains_val(wyrm_var_processed_files, wyrm_var_path))) {
    return val_array_init(0);
    }
    val_array_append(wyrm_var_processed_files, wyrm_var_path);
    Value wyrm_var_content = val_read_file(wyrm_var_path);
    if (val_to_bool(val_eq(wyrm_var_content, val_null()))) {
    wyrm_var_content = val_read_file(val_add(wyrm_var_path, val_string(".wyr")));
    }
    if (val_to_bool(val_eq(wyrm_var_content, val_null()))) {
    wyrm_var_content = val_read_file(val_add(val_string("packages/"), wyrm_var_path));
    }
    if (val_to_bool(val_eq(wyrm_var_content, val_null()))) {
    wyrm_var_content = val_read_file(val_add(val_add(val_string("packages/"), wyrm_var_path), val_string(".wyr")));
    }
    if (val_to_bool(val_eq(wyrm_var_content, val_null()))) {
    Value wyrm_var_home = val_getenv(val_string("USERPROFILE"));
    if (val_to_bool(val_eq(wyrm_var_home, val_null()))) {
    wyrm_var_home = val_getenv(val_string("HOME"));
    }
    if (val_to_bool(val_ne(wyrm_var_home, val_null()))) {
    wyrm_var_content = val_read_file(val_add(val_add(wyrm_var_home, val_string("/.wyrm/packages/")), wyrm_var_path));
    if (val_to_bool(val_eq(wyrm_var_content, val_null()))) {
    wyrm_var_content = val_read_file(val_add(val_add(val_add(wyrm_var_home, val_string("/.wyrm/packages/")), wyrm_var_path), val_string(".wyr")));
    }
    }
    }
    if (val_to_bool(val_eq(wyrm_var_content, val_null()))) {
    val_print(1, val_add(val_add(val_string("Compiler Error: Cannot find module '"), wyrm_var_path), val_string("'")));
    val_exit(val_number(1));
    }
    Value wyrm_var_tokens = wyrm_fn_tokenize(wyrm_var_content);
    val_print(2, val_string("DEBUG: READ CONTENT SIZE:"), val_len(wyrm_var_content));
    val_print(2, val_string("DEBUG: TOKENS LEN:"), val_len(wyrm_var_tokens));
    Value wyrm_var_file_ast = wyrm_fn_parse_tokens(wyrm_var_tokens);
    Value wyrm_var_full_ast = val_array_init(0);
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_l = val_len(wyrm_var_file_ast);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    Value wyrm_var_stmt = val_array_get(wyrm_var_file_ast, wyrm_var_i);
    if (val_to_bool(val_eq(val_array_get(wyrm_var_stmt, val_number(0)), val_string("Use")))) {
    Value wyrm_var_sub_ast = wyrm_fn_parse_file(val_array_get(wyrm_var_stmt, val_number(1)), wyrm_var_processed_files);
    Value wyrm_var_j = val_number(0);
    Value wyrm_var_sl = val_len(wyrm_var_sub_ast);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_sl))) {
    break;
    }
    val_array_append(wyrm_var_full_ast, val_array_get(wyrm_var_sub_ast, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_sl)));
    }
    else {
    val_array_append(wyrm_var_full_ast, wyrm_var_stmt);
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    return wyrm_var_full_ast;
    return val_null();
}

Value wyrm_fn_main() {
    Value wyrm_var_args = val_sys_args();
    val_print(2, val_string("DEBUG ARGS:"), wyrm_var_args);
    if (val_to_bool(val_lt(val_len(wyrm_var_args), val_number(3)))) {
    val_print(1, val_string("Wyrm Compiler & LLVM IR Generator (wyrmc) v2.4"));
    val_print(1, val_string("Usage:"));
    val_print(1, val_string("  wyrmc build <file.wyr>"));
    val_exit(val_number(1));
    }
    Value wyrm_var_command = val_array_get(wyrm_var_args, val_number(1));
    Value wyrm_var_source_file = val_array_get(wyrm_var_args, val_number(2));
    val_print(4, val_string("DEBUG CMD:"), wyrm_var_command, val_string("SRC:"), wyrm_var_source_file);
    if (val_to_bool(val_ne(wyrm_var_command, val_string("build")))) {
    val_print(1, val_add(val_add(val_string("Error: Unknown compiler command '"), wyrm_var_command), val_string("'")));
    val_exit(val_number(1));
    }
    Value wyrm_var_out_exe = val_string("a.exe");
    Value wyrm_var_last_dot = val_len(wyrm_var_source_file);
    Value wyrm_var_i = val_sub(val_len(wyrm_var_source_file), val_number(1));
    do {
    if (val_to_bool(val_lt(wyrm_var_i, val_number(0)))) {
    break;
    }
    Value wyrm_var_char = val_char_at(wyrm_var_source_file, wyrm_var_i);
    if (val_to_bool(val_eq(wyrm_var_char, val_string(".")))) {
    wyrm_var_last_dot = wyrm_var_i;
    break;
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_char, val_string("/")), val_eq(wyrm_var_char, val_string("\\"))))) {
    break;
    }
    wyrm_var_i = val_sub(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_lt(wyrm_var_i, val_number(0))));
    Value wyrm_var_base_name = val_string("");
    wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_last_dot))) {
    break;
    }
    wyrm_var_base_name = val_add(wyrm_var_base_name, val_char_at(wyrm_var_source_file, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_last_dot)));
    wyrm_var_out_exe = val_add(wyrm_var_base_name, val_string(".exe"));
    Value wyrm_var_temp_ll_file = val_add(wyrm_var_base_name, val_string("_temp.ll"));
    Value wyrm_var_processed = val_array_init(0);
    Value wyrm_var_ast = wyrm_fn_parse_file(wyrm_var_source_file, wyrm_var_processed);
    val_print(2, val_string("DEBUG AST:"), wyrm_var_ast);
    wyrm_fn_check_errors(wyrm_var_ast);
    Value wyrm_var_llvm_ir = wyrm_fn_transpile_llvm(wyrm_var_ast);
    Value wyrm_var_write_ok = val_write_file(wyrm_var_temp_ll_file, wyrm_var_llvm_ir);
    if (val_to_bool(val_not(wyrm_var_write_ok))) {
    val_print(1, val_add(val_add(val_string("Error: Could not write temporary LLVM IR file '"), wyrm_var_temp_ll_file), val_string("'")));
    val_exit(val_number(1));
    }
    Value wyrm_var_home = val_getenv(val_string("USERPROFILE"));
    if (val_to_bool(val_eq(wyrm_var_home, val_null()))) {
    wyrm_var_home = val_getenv(val_string("HOME"));
    }
    Value wyrm_var_lib_path = val_string("wyrm/lib/");
    Value wyrm_var_f_check = val_read_file(val_add(wyrm_var_lib_path, val_string("wyrm_core.c")));
    if (val_to_bool(val_and(val_eq(wyrm_var_f_check, val_null()), val_ne(wyrm_var_home, val_null())))) {
    wyrm_var_lib_path = val_add(wyrm_var_home, val_string("/.wyrm/packages/wyrmlang/lib/"));
    }
    Value wyrm_var_runtime_c = val_add(wyrm_var_lib_path, val_string("wyrm_core.c"));
    Value wyrm_var_arena_c = val_add(wyrm_var_lib_path, val_string("wyrm_arena.c"));
    Value wyrm_var_str_c = val_add(wyrm_var_lib_path, val_string("wyrm_str.c"));
    Value wyrm_var_compile_cmd = val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_string("\"\"C:\\Program Files\\LLVM\\bin\\clang.exe\" -O2 \""), wyrm_var_temp_ll_file), val_string("\" \"")), wyrm_var_runtime_c), val_string("\" \"")), wyrm_var_arena_c), val_string("\" \"")), wyrm_var_str_c), val_string("\" -o \"")), wyrm_var_out_exe), val_string("\"\""));
    Value wyrm_var_res = val_system(wyrm_var_compile_cmd);
    Value wyrm_var_dummy_del = wyrm_var_temp_ll_file;
    if (val_to_bool(val_ne(wyrm_var_res, val_number(0)))) {
    val_print(1, val_string("Compilation Error: LLVM/clang failed to compile target executable."));
    val_exit(val_number(1));
    }
    val_print(1, val_add(val_add(val_add(val_add(val_string("[wyrmc Compiler v2.4] Successfully compiled '"), wyrm_var_source_file), val_string("' -> Native Binary '")), wyrm_var_out_exe), val_string("' (via LLVM IR)")));
    return val_null();
}

int main(int argc, char *argv[]) {
    val_init_sys_args(argc, argv);
    wyrm_var_token_pos = val_array_init(1, val_number(0));
    wyrm_var_g_tokens = val_array_init(1, val_array_init(0));
    wyrm_var_temp_count = val_array_init(1, val_number(0));
    wyrm_var_g_allocas = val_array_init(1, val_array_init(0));
    wyrm_var_g_main_allocas = val_array_init(1, val_array_init(0));
    wyrm_var_g_insts = val_array_init(1, val_array_init(0));
    wyrm_var_g_llvm_globals = val_array_init(0);
    wyrm_var_g_llvm_decls = val_array_init(0);
    wyrm_var_g_llvm_funcs = val_array_init(0);
    wyrm_var_g_llvm_main = val_array_init(0);
    wyrm_var_g_scopes = val_array_init(1, val_array_init(0));
    wyrm_var_g_catch_labels = val_array_init(1, val_array_init(0));
    wyrm_var_g_catch_vars = val_array_init(1, val_array_init(0));
    wyrm_var_in_function = val_array_init(1, val_bool(false));
    wyrm_var_in_unsafe = val_array_init(1, val_bool(false));
    wyrm_var_has_main_def = val_array_init(1, val_bool(false));
    wyrm_var_declared_globals = val_array_init(1, val_array_init(0));
    wyrm_var_declared_locals = val_array_init(1, val_array_init(0));
    wyrm_var_constants = val_array_init(0);
    wyrm_fn_main();
    return 0;
}
