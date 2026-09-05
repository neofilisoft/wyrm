#include "wyrm_core.h"
#include "wyrm_arena.h"
#include "wyrm_str.h"
#include "wyrm_ffi.h"
#include "stdlib/wyrm_std_json.h"
#include "stdlib/wyrm_std_yaml.h"
#include "stdlib/wyrm_std_sdl.h"
#include "stdlib/wyrm_std_collections.h"
#include "stdlib/wyrm_std_random.h"

void wyrm_check_oom(void *ptr, const char *context) {
    if (!ptr) {
        fprintf(stderr, "Fatal Runtime Error: Out of memory (allocation failed in %s)\n", context);
        exit(1);
    }
}

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

Value val_error(const char *s) {
    Value v;
    v.type = VAL_ERROR;
    v.as.string = strdup(s ? s : "");
    return v;
}

Value val_error_val(Value msg) {
    char *s = val_to_str_ptr(msg);
    Value v = val_error(s);
    free(s);
    return v;
}

Value val_array_create(int count) {
    if (count < 0) {
        fprintf(stderr, "Fatal Runtime Error: val_array_create called with negative count: %d\n", count);
        exit(1);
    }
    Value v;
    v.type = VAL_ARRAY;
    v.as.array = (ValArray*)malloc(sizeof(ValArray));
    wyrm_check_oom(v.as.array, "val_array_create (ValArray struct)");
    v.as.array->size = count;
    v.as.array->capacity = count > 0 ? count : 0;
    if (count > 0) {
        size_t alloc_bytes = (size_t)count * sizeof(Value);
        v.as.array->data = (Value*)malloc(alloc_bytes);
        if (!v.as.array->data) {
            fprintf(stderr, "Fatal Runtime Error: Out of memory in val_array_create (count=%d, bytes=%zu)\n", count, alloc_bytes);
            exit(1);
        }
        // Initialize all elements to null to prevent reading garbage memory
        for (int i = 0; i < count; i++) {
            v.as.array->data[i].type = VAL_NULL;
        }
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
        case VAL_ERROR: return false;
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
        case VAL_STRING: return strdup(v.as.string ? v.as.string : "");
        case VAL_ERROR: return strdup(v.as.string ? v.as.string : "");
        case VAL_ARRAY: {
            int cap = 256;
            char *res = malloc(cap);
            wyrm_check_oom(res, "val_to_str_ptr (array formatting buffer)");
            strcpy(res, "[");
            for (int i = 0; i < v.as.array->size; i++) {
                char *item_str = val_to_str_ptr(v.as.array->data[i]);
                size_t need = strlen(res) + strlen(item_str) + 4;
                if ((int)need >= cap) {
                    cap = (int)need * 2;
                    res = realloc(res, cap);
                    wyrm_check_oom(res, "val_to_str_ptr (array formatting buffer reallocation)");
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
        case VAL_STRUCT: {
            if (!v.as.structure) return strdup("<struct null>");
            WyrmStruct *st = v.as.structure;
            int cap = 256;
            char *res = malloc(cap);
            wyrm_check_oom(res, "val_to_str_ptr (struct formatting buffer)");
            snprintf(res, cap, "%s { ", st->type_name ? st->type_name : "Struct");
            for (int i = 0; i < st->field_count; i++) {
                char *item_str = val_to_str_ptr(st->fields[i]);
                const char *fname = st->field_names && st->field_names[i] ? st->field_names[i] : "";
                int need = (int)(strlen(res) + strlen(fname) + strlen(item_str) + 8);
                if (need >= cap) {
                    cap = need * 2;
                    res = realloc(res, cap);
                    wyrm_check_oom(res, "val_to_str_ptr (struct formatting buffer reallocation)");
                }
                strcat(res, fname);
                strcat(res, ": ");
                strcat(res, item_str);
                free(item_str);
                if (i < st->field_count - 1) {
                    strcat(res, ", ");
                }
            }
            strcat(res, " }");
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
        case VAL_ERROR: return val_string("error");
    }
    return val_string("unknown");
}

Value val_int(Value v) {
    if (v.type == VAL_NUMBER) {
        return val_number(floor(v.as.number));
    } else if (v.type == VAL_BOOL) {
        return val_number(v.as.boolean ? 1.0 : 0.0);
    } else if (v.type == VAL_STRING) {
        char *end = NULL;
        double result = strtod(v.as.string, &end);
        if (end == v.as.string || *end != '\0') {
            fprintf(stderr, "Runtime Error: int() cannot convert string to a number: '%s'\n", v.as.string);
            exit(1);
        }
        return val_number(floor(result));
    }
    return val_number(0.0);
}

Value val_float(Value v) {
    if (v.type == VAL_NUMBER) {
        return v;
    } else if (v.type == VAL_BOOL) {
        return val_number(v.as.boolean ? 1.0 : 0.0);
    } else if (v.type == VAL_STRING) {
        char *end = NULL;
        double result = strtod(v.as.string, &end);
        if (end == v.as.string || *end != '\0') {
            fprintf(stderr, "Runtime Error: float() cannot convert string to a number: '%s'\n", v.as.string);
            exit(1);
        }
        return val_number(result);
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

Value val_min(int count, ...) {
    if (count <= 0) return val_null();
    va_list args;
    va_start(args, count);
    Value lowest = va_arg(args, Value);
    for (int i = 1; i < count; i++) {
        Value v = va_arg(args, Value);
        if (val_to_bool(val_lt(v, lowest))) {
            lowest = v;
        }
    }
    va_end(args);
    return lowest;
}

Value val_max(int count, ...) {
    if (count <= 0) return val_null();
    va_list args;
    va_start(args, count);
    Value highest = va_arg(args, Value);
    for (int i = 1; i < count; i++) {
        Value v = va_arg(args, Value);
        if (val_to_bool(val_gt(v, highest))) {
            highest = v;
        }
    }
    va_end(args);
    return highest;
}

Value val_add(Value a, Value b) {
    if (a.type == VAL_NUMBER && b.type == VAL_NUMBER) {
        return val_number(a.as.number + b.as.number);
    }
    char *s1 = val_to_str_ptr(a);
    char *s2 = val_to_str_ptr(b);
    char *res = malloc(strlen(s1) + strlen(s2) + 1);
    wyrm_check_oom(res, "val_add (string concatenation)");
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
        case VAL_ERROR: return val_bool(strcmp(a.as.string, b.as.string) == 0);
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
    if (index.type == VAL_STRING) {
        if (json_is_object(arr)) {
            return json_get(arr, index);
        } else if (arr.type == VAL_RAW_PTR && arr.as.raw_ptr) {
            return map_get(arr, index);
        }
    }
    if (index.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: Array/string index must be a number, got type '%s'\n",
                index.type == VAL_STRING ? "string" :
                index.type == VAL_BOOL   ? "bool"   :
                index.type == VAL_NULL   ? "null"   :
                index.type == VAL_ARRAY  ? "array"  : "unknown");
        exit(1);
    }
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
    if (index.type == VAL_STRING) {
        if (json_is_object(arr)) {
            json_set(arr, index, val);
            return val;
        } else if (arr.type == VAL_RAW_PTR && arr.as.raw_ptr) {
            map_set(arr, index, val);
            return val;
        }
    }
    if (arr.type != VAL_ARRAY) {
        fprintf(stderr, "Runtime Error: Object is not subscriptable\n");
        exit(1);
    }
    if (index.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: Array index must be a number, got type '%s'\n",
                index.type == VAL_STRING ? "string" :
                index.type == VAL_BOOL   ? "bool"   :
                index.type == VAL_NULL   ? "null"   :
                index.type == VAL_ARRAY  ? "array"  : "unknown");
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
    } else if (start.type == VAL_NUMBER) {
        s_idx = (int)start.as.number;
        if (s_idx < 0) s_idx += len;
        if (s_idx < 0) s_idx = 0;
        if (s_idx > len) s_idx = len;
    } else {
        fprintf(stderr, "Runtime Error: Slice start index must be a number or null\n");
        exit(1);
    }

    int e_idx = len;
    if (end.type == VAL_NULL) {
        e_idx = len;
    } else if (end.type == VAL_NUMBER) {
        e_idx = (int)end.as.number;
        if (e_idx < 0) e_idx += len;
        if (e_idx < 0) e_idx = 0;
        if (e_idx > len) e_idx = len;
    } else {
        fprintf(stderr, "Runtime Error: Slice end index must be a number or null\n");
        exit(1);
    }

    if (arr.type == VAL_STRING) {
        if (s_idx >= e_idx) {
            return val_string("");
        }
        int slice_len = e_idx - s_idx;
        char *buf = malloc(slice_len + 1);
        wyrm_check_oom(buf, "val_array_slice (string slice buffer)");
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
            res.as.array->data[i] = val_copy(arr.as.array->data[s_idx + i]);
        }
        return res;
    }
}

// -------------------------------------------------------------------------
// val_drop: Destructor for a Value. Frees all heap-allocated resources.
// The caller must pass a Value it owns. No-op for scalar types.
// -------------------------------------------------------------------------
void val_drop(Value v) {
    switch (v.type) {
        case VAL_STRING:
        case VAL_ERROR:
            free(v.as.string);
            break;
        case VAL_ARRAY:
            if (v.as.array) {
                // Recursively drop every element the array owns
                for (int i = 0; i < v.as.array->size; i++) {
                    val_drop(v.as.array->data[i]);
                }
                free(v.as.array->data);
                free(v.as.array);
            }
            break;
        case VAL_STRUCT:
            if (v.as.structure) {
                WyrmStruct *st = v.as.structure;
                st->ref_count--;
                if (st->ref_count <= 0) {
                    if (st->fields) {
                        for (int i = 0; i < st->field_count; i++) {
                            val_drop(st->fields[i]);
                        }
                        free(st->fields);
                    }
                    if (st->field_names) {
                        for (int i = 0; i < st->field_count; i++) {
                            if (st->field_names[i]) free(st->field_names[i]);
                        }
                        free(st->field_names);
                    }
                    if (st->type_name) {
                        free(st->type_name);
                    }
                    free(st);
                }
            }
            break;
        case VAL_NULL:
        case VAL_BOOL:
        case VAL_NUMBER:
        case VAL_RAW_PTR:
            // Scalar types have no heap allocation managed by this runtime
            break;
    }
}

// -------------------------------------------------------------------------
// val_copy: Deep copy a Value. Returns a new independently owned Value.
// The caller is responsible for calling val_drop on the returned Value.
// -------------------------------------------------------------------------
Value val_copy(Value v) {
    switch (v.type) {
        case VAL_STRING:
            return val_string(v.as.string ? v.as.string : "");
        case VAL_ERROR:
            return val_error(v.as.string ? v.as.string : "");
        case VAL_ARRAY: {
            if (!v.as.array) return val_array_create(0);
            Value copy = val_array_create(v.as.array->size);
            for (int i = 0; i < v.as.array->size; i++) {
                copy.as.array->data[i] = val_copy(v.as.array->data[i]);
            }
            return copy;
        }
        case VAL_STRUCT: {
            if (!v.as.structure) return val_null();
            v.as.structure->ref_count++;
            return v;
        }
        case VAL_NULL:
        case VAL_BOOL:
        case VAL_NUMBER:
        case VAL_RAW_PTR:
        default:
            return v; // Scalar types are trivially copied by value
    }
}

// -------------------------------------------------------------------------
// Struct Operations
// -------------------------------------------------------------------------
Value val_struct_create(const char *type_name, int field_count, const char **field_names, const Value *initial_fields) {
    WyrmStruct *s = (WyrmStruct *)malloc(sizeof(WyrmStruct));
    wyrm_check_oom(s, "val_struct_create (struct header)");
    s->type_name = strdup(type_name ? type_name : "Struct");
    wyrm_check_oom(s->type_name, "val_struct_create (type_name)");
    s->field_count = field_count;
    s->ref_count = 1;
    if (field_count > 0) {
        s->field_names = (char **)malloc(sizeof(char *) * (size_t)field_count);
        wyrm_check_oom(s->field_names, "val_struct_create (field_names)");
        s->fields = (Value *)malloc(sizeof(Value) * (size_t)field_count);
        wyrm_check_oom(s->fields, "val_struct_create (fields)");
        for (int i = 0; i < field_count; i++) {
            s->field_names[i] = strdup(field_names && field_names[i] ? field_names[i] : "");
            wyrm_check_oom(s->field_names[i], "val_struct_create (field_name string)");
            s->fields[i] = initial_fields ? initial_fields[i] : val_null();
        }
    } else {
        s->field_names = NULL;
        s->fields = NULL;
    }
    Value res;
    res.type = VAL_STRUCT;
    res.as.structure = s;
    return res;
}

Value val_struct_get(Value s, const char *field_name) {
    if (s.type != VAL_STRUCT || !s.as.structure || !field_name) {
        return val_null();
    }
    WyrmStruct *st = s.as.structure;
    for (int i = 0; i < st->field_count; i++) {
        if (st->field_names[i] && strcmp(st->field_names[i], field_name) == 0) {
            return val_copy(st->fields[i]);
        }
    }
    return val_null();
}

Value val_struct_set(Value s, const char *field_name, Value new_val) {
    if (s.type != VAL_STRUCT || !s.as.structure || !field_name) {
        return s;
    }
    WyrmStruct *st = s.as.structure;
    for (int i = 0; i < st->field_count; i++) {
        if (st->field_names[i] && strcmp(st->field_names[i], field_name) == 0) {
            val_drop(st->fields[i]);
            st->fields[i] = val_copy(new_val);
            return s;
        }
    }
    for (int i = 0; i < st->field_count; i++) {
        if (st->field_names[i] && st->field_names[i][0] == '\0') {
            free(st->field_names[i]);
            st->field_names[i] = strdup(field_name);
            val_drop(st->fields[i]);
            st->fields[i] = val_copy(new_val);
            return s;
        }
    }
    return s;
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
        void *new_data = realloc(arr.as.array->data, new_cap * sizeof(Value));
        wyrm_check_oom(new_data, "val_array_append (array data reallocation)");
        arr.as.array->data = (Value*)new_data;
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
    if (size.as.number < 0.0) {
        fprintf(stderr, "Runtime Error: malloc size cannot be negative: %g\n", size.as.number);
        exit(1);
    }
    Value v;
    v.type = VAL_RAW_PTR;
    v.as.raw_ptr = malloc((size_t)size.as.number);
    if (size.as.number > 0) {
        wyrm_check_oom(v.as.raw_ptr, "val_raw_malloc");
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
    if (size.as.number < 0.0) {
        fprintf(stderr, "Runtime Error: realloc size cannot be negative: %g\n", size.as.number);
        exit(1);
    }
    Value v;
    v.type = VAL_RAW_PTR;
    void *old_ptr = (ptr.type == VAL_RAW_PTR) ? ptr.as.raw_ptr : NULL;
    v.as.raw_ptr = realloc(old_ptr, (size_t)size.as.number);
    if (size.as.number > 0) {
        wyrm_check_oom(v.as.raw_ptr, "val_raw_realloc");
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

Value val_arena_alloc(WyrmArena *a, Value size) {
    if (size.type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: arena alloc size must be a number\n");
        return val_null();
    }
    void *ptr = arena_alloc(a, (size_t)size.as.number);
    Value v;
    v.type = VAL_RAW_PTR;
    v.as.raw_ptr = ptr;
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
    if (size < 0) {
        fclose(f);
        return val_error("Cannot determine file size (non-seekable file)");
    }
    fseek(f, 0, SEEK_SET);

    char *buf = malloc(size + 1);
    wyrm_check_oom(buf, "val_read_file (file read buffer)");
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
void llvm_val_error(Value *res, const char *s) { *res = val_error(s); }
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
void llvm_val_arena_alloc(Value *res, WyrmArena *a, Value *size) { *res = val_arena_alloc(a, *size); }
void llvm_val_arena_reset(Value *res, WyrmArena *a) { *res = val_arena_reset(a); }

void llvm_val_read_file(Value *res, Value *path) { *res = val_read_file(*path); }
void llvm_val_error_val(Value *res, Value *msg) { *res = val_error_val(*msg); }
void llvm_val_write_file(Value *res, Value *path, Value *content) { *res = val_write_file(*path, *content); }
void llvm_val_exit(Value *res, Value *code) { *res = val_exit(*code); }
void llvm_val_system(Value *res, Value *cmd) { *res = val_system(*cmd); }
void llvm_val_getenv(Value *res, Value *name) { *res = val_getenv(*name); }

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
void llvm_val_copy(Value *dest, Value *src) { *dest = val_copy(*src); }

WyrmArena* val_arena_create_wrapper(Value *size) {
    if (size->type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: arena size must be a number\n");
        exit(1);
    }
    return arena_create((size_t)size->as.number);
}

void llvm_val_struct_create(Value *res, const char *type_name, int field_count) {
    *res = val_struct_create(type_name, field_count, NULL, NULL);
}

void llvm_val_struct_get(Value *res, Value *s, const char *field_name) {
    *res = val_struct_get(*s, field_name);
}

void llvm_val_struct_set(Value *s, const char *field_name, Value *val) {
    *s = val_struct_set(*s, field_name, *val);
}

void llvm_val_from_i64(Value *res, int64_t v) {
    *res = val_number((double)v);
}

void llvm_val_from_u8(Value *res, uint8_t v) {
    *res = val_number((double)v);
}

void llvm_val_from_f32(Value *res, float v) {
    *res = val_number((double)v);
}

void llvm_val_from_bool(Value *res, bool v) {
    *res = val_bool(v);
}

void llvm_val_drop(Value *v) {
    val_drop(*v);
}

// Standard library wrappers
void llvm_val_json_parse(Value *res, Value *s) { *res = json_parse(*s); }
void llvm_val_json_encode(Value *res, Value *v) { *res = json_encode(*v); }
void llvm_val_json_pretty(Value *res, Value *v, Value *indent) { *res = json_pretty(*v, *indent); }
void llvm_val_json_get(Value *res, Value *obj, Value *key) { *res = json_get(*obj, *key); }
void llvm_val_json_has(Value *res, Value *obj, Value *key) { *res = json_has(*obj, *key); }
void llvm_val_json_set(Value *res, Value *obj, Value *key, Value *val) { *res = json_set(*obj, *key, *val); }
void llvm_val_json_object(Value *res) { *res = json_object(); }

void llvm_val_yaml_parse(Value *res, Value *s) { *res = yaml_parse(*s); }
void llvm_val_yaml_encode(Value *res, Value *v) { *res = yaml_encode(*v); }

void llvm_val_map_new(Value *res) { *res = map_new(); }
void llvm_val_map_set(Value *res, Value *m, Value *k, Value *v) { *res = map_set(*m, *k, *v); }
void llvm_val_map_get(Value *res, Value *m, Value *k) { *res = map_get(*m, *k); }
void llvm_val_map_has(Value *res, Value *m, Value *k) { *res = map_has(*m, *k); }
void llvm_val_map_del(Value *res, Value *m, Value *k) { *res = map_del(*m, *k); }
void llvm_val_map_keys(Value *res, Value *m) { *res = map_keys(*m); }
void llvm_val_map_values(Value *res, Value *m) { *res = map_values(*m); }
void llvm_val_map_len(Value *res, Value *m) { *res = map_len(*m); }

void llvm_val_set_new(Value *res) { *res = set_new(); }
void llvm_val_set_add(Value *res, Value *s, Value *v) { *res = set_add(*s, *v); }
void llvm_val_set_has(Value *res, Value *s, Value *v) { *res = set_has(*s, *v); }
void llvm_val_set_del(Value *res, Value *s, Value *v) { *res = set_del(*s, *v); }
void llvm_val_set_union(Value *res, Value *a, Value *b) { *res = set_union_fn(*a, *b); }
void llvm_val_set_intersect(Value *res, Value *a, Value *b) { *res = set_intersect(*a, *b); }
void llvm_val_set_to_array(Value *res, Value *s) { *res = set_to_array(*s); }

void llvm_val_sdl_init(Value *res) { *res = sdl_init(); }
void llvm_val_sdl_quit(Value *res) { *res = sdl_quit(); }
void llvm_val_sdl_window(Value *res, Value *title, Value *w, Value *h) { *res = sdl_window(*title, *w, *h); }
void llvm_val_sdl_destroy_window(Value *res, Value *win) { *res = sdl_destroy_window(*win); }
void llvm_val_sdl_poll_event(Value *res) { *res = sdl_poll_event(); }
void llvm_val_sdl_clear(Value *res, Value *win, Value *r, Value *g, Value *b) { *res = sdl_clear(*win, *r, *g, *b); }
void llvm_val_sdl_present(Value *res, Value *win) { *res = sdl_present(*win); }
void llvm_val_sdl_draw_rect(Value *res, Value *win, Value *x, Value *y, Value *w, Value *h, Value *r, Value *g, Value *b) { *res = sdl_draw_rect(*win, *x, *y, *w, *h, *r, *g, *b); }
void llvm_val_sdl_draw_line(Value *res, Value *win, Value *x1, Value *y1, Value *x2, Value *y2, Value *r, Value *g, Value *b) { *res = sdl_draw_line(*win, *x1, *y1, *x2, *y2, *r, *g, *b); }
void llvm_val_sdl_delay(Value *res, Value *ms) { *res = sdl_delay(*ms); }
void llvm_val_sdl_ticks(Value *res) { *res = sdl_ticks(); }

void llvm_val_ffi_open(Value *res, Value *path) { *res = ffi_open(*path); }
void llvm_val_ffi_sym(Value *res, Value *lib, Value *sym) { *res = ffi_sym(*lib, *sym); }
void llvm_val_ffi_call(Value *res, Value *fn_ptr, Value *args) { *res = ffi_call_fn(*fn_ptr, *args); }
void llvm_val_ffi_close(Value *res, Value *lib) { *res = ffi_close(*lib); }

void llvm_val_rand_seed(Value *res, Value *seed) { *res = rand_seed(*seed); }
void llvm_val_rand(Value *res) { *res = rand_val(); }
void llvm_val_rand_int(Value *res, Value *min, Value *max) { *res = rand_int(*min, *max); }
void llvm_val_rand_range(Value *res, Value *min, Value *max) { *res = rand_range(*min, *max); }
void llvm_val_rand_choice(Value *res, Value *arr) { *res = rand_choice(*arr); }
void llvm_val_rand_shuffle(Value *res, Value *arr) { *res = rand_shuffle(*arr); }
void llvm_val_rand_secure(Value *res) { *res = rand_secure(); }
void llvm_val_rand_secure_int(Value *res, Value *min, Value *max) { *res = rand_secure_int(*min, *max); }
void llvm_val_rand_bytes_hex(Value *res, Value *count) { *res = rand_bytes_hex(*count); }
void llvm_val_rand_has_trng(Value *res) { *res = rand_has_trng(); }
void llvm_val_rand_trng(Value *res) { *res = rand_trng(); }
void llvm_val_rand_trng_int(Value *res, Value *min, Value *max) { *res = rand_trng_int(*min, *max); }
void llvm_val_rand_reseed_trng(Value *res) { *res = rand_reseed_trng(); }

