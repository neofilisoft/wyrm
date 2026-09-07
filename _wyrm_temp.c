
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
    VAL_ERROR,
    VAL_WEAK_REF
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
    int weak_count;
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
Value val_weak_ref(Value st);
Value val_weak_lock(Value w);

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

void llvm_val_rand_seed(Value *res, Value *seed);
void llvm_val_rand(Value *res);
void llvm_val_rand_int(Value *res, Value *min, Value *max);
void llvm_val_rand_range(Value *res, Value *min, Value *max);
void llvm_val_rand_choice(Value *res, Value *arr);
void llvm_val_rand_shuffle(Value *res, Value *arr);
void llvm_val_rand_secure(Value *res);
void llvm_val_rand_secure_int(Value *res, Value *min, Value *max);
void llvm_val_rand_bytes_hex(Value *res, Value *count);
void llvm_val_rand_has_trng(Value *res);
void llvm_val_rand_trng(Value *res);
void llvm_val_rand_trng_int(Value *res, Value *min, Value *max);
void llvm_val_rand_reseed_trng(Value *res);

void llvm_val_time_now(Value *res);
void llvm_val_time_unix(Value *res);
void llvm_val_time_unix_ms(Value *res);
void llvm_val_time_monotonic(Value *res);
void llvm_val_time_monotonic_ms(Value *res);
void llvm_val_time_monotonic_ns(Value *res);
void llvm_val_time_sleep(Value *res, Value *ms);
void llvm_val_time_diff(Value *res, Value *start, Value *end);
void llvm_val_time_format(Value *res, Value *ts, Value *fmt);
void llvm_val_time_format_local(Value *res, Value *ts, Value *fmt);

void llvm_val_weak_ref(Value *res, Value *st);
void llvm_val_weak_lock(Value *res, Value *w);


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

/*
 * wyrm_ffi.h - Wyrm Foreign Function Interface
 *
 * Provides dynamic shared library loading and the Wyrm Extension ABI.
 * External C libraries expose functions using this calling convention:
 *
 *   typedef Value (*WyrmExtFunc)(int argc, Value *argv);
 *
 * This is the same model Lua uses for its C API: clean, portable, and
 * requires no libffi dependency.
 */


#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------
 * Wyrm Extension ABI
 * --------------------------------------------------------------------------
 * Every function exported from a Wyrm extension library must follow this
 * signature. argc is the argument count; argv is the Value argument array.
 * The function returns a single Value result.
 */
typedef Value (*WyrmExtFunc)(int argc, Value *argv);

/* --------------------------------------------------------------------------
 * WyrmFfiLib - Handle to a loaded shared library
 * --------------------------------------------------------------------------
 * Stored in a Value as VAL_RAW_PTR. Freed by ffi_close().
 */
typedef struct WyrmFfiLib {
    void *handle;   /* dlopen/LoadLibrary handle                          */
    char *path;     /* heap-allocated resolved path string                 */
} WyrmFfiLib;

/* --------------------------------------------------------------------------
 * Runtime FFI builtins (exposed as Wyrm built-in functions)
 * --------------------------------------------------------------------------
 *
 * ffi_open(path)
 *   Opens the shared library at 'path'. Returns a VAL_RAW_PTR(WyrmFfiLib).
 *   Exits with a runtime error if the library cannot be loaded.
 *
 * ffi_sym(lib, symbol)
 *   Resolves a symbol name from a library handle.
 *   The resolved symbol must be a WyrmExtFunc-compatible function pointer.
 *   Returns a VAL_RAW_PTR(WyrmExtFunc).
 *
 * ffi_call(func, args_array)
 *   Calls the resolved WyrmExtFunc with arguments from the Wyrm array.
 *   Returns whatever Value the function returns.
 *
 * ffi_close(lib)
 *   Unloads the shared library and frees the WyrmFfiLib struct.
 *   Returns VAL_NULL.
 */
Value ffi_open(Value path_val);
Value ffi_sym(Value lib_val, Value symbol_val);
Value ffi_call_fn(Value func_val, Value args_val);
Value ffi_close(Value lib_val);

#ifdef __cplusplus
}
#endif

/*
 * wyrm_std_json.h - Wyrm Standard Library: JSON Serialization
 *
 * Pure C11 recursive-descent JSON parser and encoder.
 * No external dependencies.
 *
 * Wyrm builtins provided (registered when `use std.json` is encountered):
 *   json_parse(str)             -> Value (array/map represented as nested arrays)
 *   json_encode(val)            -> string
 *   json_pretty(val, indent)    -> string (pretty-printed with indent spaces)
 */


#ifdef __cplusplus
extern "C" {
#endif

/*
 * JSON Value Representation in Wyrm:
 *   JSON null      -> VAL_NULL
 *   JSON bool      -> VAL_BOOL
 *   JSON number    -> VAL_NUMBER
 *   JSON string    -> VAL_STRING
 *   JSON array     -> VAL_ARRAY of Values
 *   JSON object    -> VAL_ARRAY of alternating key-value pairs:
 *                     [key0, val0, key1, val1, ...]
 *                     tagged with a sentinel string "__json_obj__" at index 0
 *
 * Object representation (array with sentinel):
 *   arr[0] = "__json_obj__"
 *   arr[1] = key0 (VAL_STRING)
 *   arr[2] = val0 (any JSON value)
 *   arr[3] = key1 (VAL_STRING)
 *   ...
 *
 * Wyrm code accesses object fields via json_get(obj, key).
 */

/* Parse a JSON string. Returns the parsed Value tree or VAL_ERROR on failure. */
Value json_parse(Value json_str);

/* Encode a Value tree to a compact JSON string. */
Value json_encode(Value val);

/* Encode a Value tree to a pretty-printed JSON string with the given indent size. */
Value json_pretty(Value val, Value indent_size);

/* Object field access: json_get(obj, "key") */
Value json_get(Value obj, Value key);

/* Check if an object has a key: json_has(obj, "key") -> bool */
Value json_has(Value obj, Value key);

/* Set a field on a JSON object: json_set(obj, "key", value) */
Value json_set(Value obj, Value key, Value val);

/* Create an empty JSON object */
Value json_object(void);

/* Check if a Value is a JSON object (has the sentinel) */
int json_is_object(Value v);

#ifdef __cplusplus
}
#endif

/*
 * wyrm_std_yaml.h - Wyrm Standard Library: YAML Serialization
 *
 * Pure C11 YAML subset parser (block style) and encoder.
 * Covers: scalars, sequences (- item), mappings (key: value),
 * multi-level nesting, quoted strings, null/bool/number detection.
 * No external dependencies.
 *
 * YAML objects are stored identically to JSON objects (sentinel-tagged arrays)
 * for unified access through json_get/json_set.
 */


#ifdef __cplusplus
extern "C" {
#endif

/*
 * Parse YAML text into a Wyrm Value tree.
 * Uses the same object representation as the JSON module.
 * Returns VAL_ERROR on parse failure.
 */
Value yaml_parse(Value yaml_str);

/*
 * Encode a Wyrm Value tree to YAML text.
 * Produces block-style YAML with 2-space indentation.
 */
Value yaml_encode(Value val);

#ifdef __cplusplus
}
#endif

/*
 * wyrm_std_sdl.h - Wyrm Standard Library: SDL2 Window & Input Binding
 *
 * Runtime-loaded SDL2 binding via dlopen/LoadLibrary.
 * SDL2 does NOT need to be linked at compile time - only SDL2.dll or
 * libSDL2.so needs to be present at runtime. The path is resolved by:
 *   1. WYRM_SDL2_PATH environment variable
 *   2. Platform default name (SDL2.dll / libSDL2.so / libSDL2-2.0.0.dylib)
 *
 * Wyrm SDL2 event map keys:
 *   "type"     -> string: "quit", "keydown", "keyup", "mousemotion",
 *                          "mousedown", "mouseup", "none"
 *   "key"      -> string: SDL key name (e.g. "Escape", "Space", "Up")
 *   "scancode" -> number: SDL scancode integer
 *   "mouse_x"  -> number: mouse X position
 *   "mouse_y"  -> number: mouse Y position
 *   "button"   -> number: mouse button index
 */


#ifdef __cplusplus
extern "C" {
#endif

/* Initialize SDL2 (loads the SDL2 library). Must be called first.
 * Returns null on success, error string on failure.        */
Value sdl_init(void);

/* Shut down SDL2 and release resources. */
Value sdl_quit(void);

/* Create a window.
 *   title     - window title string
 *   width     - window width in pixels
 *   height    - window height in pixels
 * Returns a VAL_RAW_PTR(WyrmSdlWindow) or error.          */
Value sdl_window(Value title, Value width, Value height);

/* Destroy a window created by sdl_window. */
Value sdl_destroy_window(Value win_val);

/* Poll the next event from the SDL event queue.
 * Returns a JSON-style object map with event data.
 * Returns an object with type="none" if no event is pending. */
Value sdl_poll_event(void);

/* Clear the window to the given RGB color (0-255 each). */
Value sdl_clear(Value win_val, Value r, Value g, Value b);

/* Present (flip) the window back buffer. */
Value sdl_present(Value win_val);

/* Draw a filled rectangle. */
Value sdl_draw_rect(Value win_val, Value x, Value y, Value w, Value h,
                    Value r, Value g, Value b);

/* Draw a line. */
Value sdl_draw_line(Value win_val, Value x1, Value y1, Value x2, Value y2,
                    Value r, Value g, Value b);

/* Delay execution by ms milliseconds (useful for frame rate control). */
Value sdl_delay(Value ms);

/* Return the number of milliseconds since SDL initialization. */
Value sdl_ticks(void);

/* SDL event type constants as Values - used in Wyrm comparisons */
extern const char *const WYRM_SDL_EVT_QUIT;
extern const char *const WYRM_SDL_EVT_KEYDOWN;
extern const char *const WYRM_SDL_EVT_KEYUP;
extern const char *const WYRM_SDL_EVT_MOUSEMOTION;
extern const char *const WYRM_SDL_EVT_MOUSEDOWN;
extern const char *const WYRM_SDL_EVT_MOUSEUP;
extern const char *const WYRM_SDL_EVT_NONE;

#ifdef __cplusplus
}
#endif

/*
 * wyrm_std_collections.h - Wyrm Standard Library: Collections
 *
 * HashMap and Set implementations over Wyrm Values.
 * Keys are VAL_STRING only for v2.6.0 (covers 95% of real use cases).
 *
 * HashMap is stored as a VAL_RAW_PTR(WyrmMap).
 * Set is stored as a VAL_RAW_PTR(WyrmSet).
 */


#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------
 * HashMap: string -> Value
 * -------------------------------------------------------------------------- */

/* Create an empty map. Returns VAL_RAW_PTR(WyrmMap). */
Value map_new(void);

/* Set key (string) to value. Returns the map handle (same pointer). */
Value map_set(Value map_val, Value key, Value val);

/* Get value for key. Returns VAL_NULL if key not found. */
Value map_get(Value map_val, Value key);

/* Returns VAL_BOOL true if key exists. */
Value map_has(Value map_val, Value key);

/* Delete key. Returns VAL_NULL. */
Value map_del(Value map_val, Value key);

/* Returns VAL_ARRAY of all keys (strings). */
Value map_keys(Value map_val);

/* Returns VAL_ARRAY of all values. */
Value map_values(Value map_val);

/* Returns VAL_NUMBER of entries. */
Value map_len(Value map_val);

/* Free a map created by map_new. */
Value map_free(Value map_val);

/* --------------------------------------------------------------------------
 * Set: string values (unordered, unique)
 * -------------------------------------------------------------------------- */

/* Create an empty set. Returns VAL_RAW_PTR(WyrmSet). */
Value set_new(void);

/* Add value to set. Returns the set handle. */
Value set_add(Value set_val, Value item);

/* Returns VAL_BOOL true if item is in set. */
Value set_has(Value set_val, Value item);

/* Remove item from set. Returns VAL_NULL. */
Value set_del(Value set_val, Value item);

/* Union of two sets. Returns a new set. */
Value set_union_fn(Value a, Value b);

/* Intersection of two sets. Returns a new set. */
Value set_intersect(Value a, Value b);

/* Convert set to a VAL_ARRAY. */
Value set_to_array(Value set_val);

/* Free a set. */
Value set_free(Value set_val);

#ifdef __cplusplus
}
#endif

/*
 * wyrm_std_random.h - Wyrm Standard Library: Random Number Generation
 *
 * Provides PRNG (Xoshiro256**), CSPRNG (OS Cryptographic entropy),
 * and TRNG (CPU Hardware RDRAND / RDSEED with OS entropy fallback).
 *
 * Wyrm builtins provided (registered when `use std.random;` is encountered):
 *   rand_seed(val)              -> VAL_NULL (Seeds PRNG; auto-seeds if null)
 *   rand()                      -> VAL_NUMBER [0.0, 1.0)
 *   rand_int(min, max)          -> VAL_NUMBER integer in [min, max] inclusive
 *   rand_range(min, max)        -> VAL_NUMBER integer in [min, max] inclusive
 *   rand_choice(array)          -> Value (Random element from array)
 *   rand_shuffle(array)         -> VAL_ARRAY (New shuffled copy via Fisher-Yates)
 *   rand_secure()               -> VAL_NUMBER [0.0, 1.0) (Cryptographically secure)
 *   rand_secure_int(min, max)   -> VAL_NUMBER unbiased integer in [min, max]
 *   rand_bytes_hex(count)       -> VAL_STRING hex string of random bytes
 *   rand_has_trng()             -> VAL_BOOL true if CPU RDRAND is supported
 *   rand_trng()                 -> VAL_NUMBER [0.0, 1.0) via hardware TRNG
 *   rand_trng_int(min, max)     -> VAL_NUMBER integer in [min, max] via TRNG
 *   rand_reseed_trng()          -> VAL_NULL (Reseeds PRNG state from TRNG)
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize random subsystem */
void wyrm_random_init(void);

/* PRNG functions */
Value rand_seed(Value seed_val);
Value rand_val(void);
Value rand_int(Value min_val, Value max_val);
Value rand_range(Value min_val, Value max_val);
Value rand_choice(Value arr_val);
Value rand_shuffle(Value arr_val);

/* CSPRNG functions */
Value rand_secure(void);
Value rand_secure_int(Value min_val, Value max_val);
Value rand_bytes_hex(Value count_val);

/* TRNG functions */
Value rand_has_trng(void);
Value rand_trng(void);
Value rand_trng_int(Value min_val, Value max_val);
Value rand_reseed_trng(void);

#ifdef __cplusplus
}
#endif

/*
 * wyrm_std_time.h - Wyrm Standard Library: High-Resolution Time Subsystem
 *
 * Provides high-resolution monotonic timers (Windows QPC / POSIX CLOCK_MONOTONIC),
 * Unix epoch wall-clock timestamps, millisecond thread sleep, and date/time formatting.
 *
 * Wyrm builtins provided (registered when `use std.time;` is encountered):
 *   time_now()                  -> VAL_NUMBER (floating-point seconds since Unix epoch)
 *   time_unix()                 -> VAL_NUMBER (integer seconds since Unix epoch)
 *   time_unix_ms()              -> VAL_NUMBER (integer milliseconds since Unix epoch)
 *   time_monotonic()            -> VAL_NUMBER (high-precision monotonic seconds)
 *   time_monotonic_ms()         -> VAL_NUMBER (high-precision monotonic milliseconds)
 *   time_monotonic_ns()         -> VAL_NUMBER (high-precision monotonic nanoseconds)
 *   time_sleep(ms)              -> VAL_NULL (sleeps current thread for ms milliseconds)
 *   time_diff(start, end)       -> VAL_NUMBER (end - start in seconds)
 *   time_format(ts, fmt)        -> VAL_STRING (formatted UTC date/time string)
 *   time_format_local(ts, fmt)  -> VAL_STRING (formatted local date/time string)
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize time subsystem (e.g. QPC frequency caching) */
void wyrm_time_init(void);

/* Wall-clock timestamps */
Value time_now(void);
Value time_unix(void);
Value time_unix_ms(void);

/* High-resolution monotonic timers */
Value time_monotonic(void);
Value time_monotonic_ms(void);
Value time_monotonic_ns(void);

/* Execution control & utility */
Value time_sleep_ms(Value ms_val);
Value time_diff(Value start_val, Value end_val);

/* Formatting */
Value time_format_utc(Value ts_val, Value fmt_val);
Value time_format_local(Value ts_val, Value fmt_val);

#ifdef __cplusplus
}
#endif


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
        case VAL_STRUCT: return v.as.structure != NULL && v.as.structure->ref_count > 0;
        case VAL_WEAK_REF: return v.as.structure != NULL && v.as.structure->ref_count > 0 && v.as.structure->fields != NULL;
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
        case VAL_WEAK_REF: {
            if (!v.as.structure || v.as.structure->ref_count <= 0 || !v.as.structure->fields) {
                return strdup("<weak null>");
            }
            char wbuf[128];
            snprintf(wbuf, sizeof(wbuf), "<weak %s at %p>", v.as.structure->type_name ? v.as.structure->type_name : "Struct", (void*)v.as.structure);
            return strdup(wbuf);
        }
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
        case VAL_STRUCT: return val_string("struct");
        case VAL_WEAK_REF: return val_string("weak_ref");
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
    if (a.type == VAL_WEAK_REF && b.type == VAL_NULL) {
        bool is_dead = (!a.as.structure || a.as.structure->ref_count <= 0 || !a.as.structure->fields);
        return val_bool(is_dead);
    }
    if (a.type == VAL_NULL && b.type == VAL_WEAK_REF) {
        bool is_dead = (!b.as.structure || b.as.structure->ref_count <= 0 || !b.as.structure->fields);
        return val_bool(is_dead);
    }
    if (a.type == VAL_WEAK_REF && b.type == VAL_STRUCT) {
        return val_bool(a.as.structure == b.as.structure && a.as.structure && a.as.structure->ref_count > 0 && a.as.structure->fields != NULL);
    }
    if (a.type == VAL_STRUCT && b.type == VAL_WEAK_REF) {
        return val_bool(a.as.structure == b.as.structure && a.as.structure && a.as.structure->ref_count > 0 && a.as.structure->fields != NULL);
    }
    if (a.type != b.type) return val_bool(false);
    switch (a.type) {
        case VAL_NULL: return val_bool(true);
        case VAL_BOOL: return val_bool(a.as.boolean == b.as.boolean);
        case VAL_NUMBER: return val_bool(a.as.number == b.as.number);
        case VAL_STRING: return val_bool(strcmp(a.as.string, b.as.string) == 0);
        case VAL_ARRAY: return val_bool(a.as.array == b.as.array);
        case VAL_STRUCT: return val_bool(a.as.structure == b.as.structure);
        case VAL_WEAK_REF: return val_bool(a.as.structure == b.as.structure);
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
                        st->fields = NULL;
                    }
                    if (st->field_names) {
                        for (int i = 0; i < st->field_count; i++) {
                            if (st->field_names[i]) free(st->field_names[i]);
                        }
                        free(st->field_names);
                        st->field_names = NULL;
                    }
                    if (st->type_name) {
                        free(st->type_name);
                        st->type_name = NULL;
                    }
                    st->field_count = 0;
                    if (st->weak_count <= 0) {
                        free(st);
                    }
                }
            }
            break;
        case VAL_WEAK_REF:
            if (v.as.structure) {
                WyrmStruct *st = v.as.structure;
                st->weak_count--;
                if (st->weak_count <= 0 && st->ref_count <= 0) {
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
        case VAL_WEAK_REF: {
            if (!v.as.structure) return val_null();
            v.as.structure->weak_count++;
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
    s->weak_count = 0;
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
    if (s.type == VAL_WEAK_REF) {
        Value locked = val_weak_lock(s);
        if (locked.type == VAL_NULL) {
            return val_null();
        }
        Value res = val_struct_get(locked, field_name);
        val_drop(locked);
        return res;
    }
    if (s.type != VAL_STRUCT || !s.as.structure || !field_name) {
        return val_null();
    }
    WyrmStruct *st = s.as.structure;
    if (st->ref_count <= 0 || !st->fields) {
        return val_null();
    }
    for (int i = 0; i < st->field_count; i++) {
        if (st->field_names[i] && strcmp(st->field_names[i], field_name) == 0) {
            return val_copy(st->fields[i]);
        }
    }
    return val_null();
}

Value val_struct_set(Value s, const char *field_name, Value new_val) {
    if (s.type == VAL_WEAK_REF) {
        Value locked = val_weak_lock(s);
        if (locked.type != VAL_NULL) {
            val_struct_set(locked, field_name, new_val);
            val_drop(locked);
        }
        return s;
    }
    if (s.type != VAL_STRUCT || !s.as.structure || !field_name) {
        return s;
    }
    WyrmStruct *st = s.as.structure;
    if (st->ref_count <= 0 || !st->fields) {
        return s;
    }
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

// -------------------------------------------------------------------------
// Weak Reference Operations
// -------------------------------------------------------------------------
Value val_weak_ref(Value st) {
    if (st.type == VAL_WEAK_REF) {
        return val_copy(st);
    }
    if (st.type != VAL_STRUCT || !st.as.structure) {
        return val_null();
    }
    st.as.structure->weak_count++;
    Value w;
    w.type = VAL_WEAK_REF;
    w.as.structure = st.as.structure;
    return w;
}

Value val_weak_lock(Value w) {
    if (w.type == VAL_STRUCT) {
        return val_copy(w);
    }
    if (w.type != VAL_WEAK_REF || !w.as.structure || w.as.structure->ref_count <= 0 || !w.as.structure->fields) {
        return val_null();
    }
    w.as.structure->ref_count++;
    Value res;
    res.type = VAL_STRUCT;
    res.as.structure = w.as.structure;
    return res;
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

void llvm_val_weak_ref(Value *res, Value *st) {
    *res = val_weak_ref(*st);
}

void llvm_val_weak_lock(Value *res, Value *w) {
    *res = val_weak_lock(*w);
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

void llvm_val_time_now(Value *res) { *res = time_now(); }
void llvm_val_time_unix(Value *res) { *res = time_unix(); }
void llvm_val_time_unix_ms(Value *res) { *res = time_unix_ms(); }
void llvm_val_time_monotonic(Value *res) { *res = time_monotonic(); }
void llvm_val_time_monotonic_ms(Value *res) { *res = time_monotonic_ms(); }
void llvm_val_time_monotonic_ns(Value *res) { *res = time_monotonic_ns(); }
void llvm_val_time_sleep(Value *res, Value *ms) { *res = time_sleep_ms(*ms); }
void llvm_val_time_diff(Value *res, Value *start, Value *end) { *res = time_diff(*start, *end); }
void llvm_val_time_format(Value *res, Value *ts, Value *fmt) { *res = time_format_utc(*ts, *fmt); }
void llvm_val_time_format_local(Value *res, Value *ts, Value *fmt) { *res = time_format_local(*ts, *fmt); }
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- Arena Allocator Implementation ----

WyrmArena *arena_create(size_t cap) {
    WyrmArena *a = (WyrmArena*)malloc(sizeof(WyrmArena));
    wyrm_check_oom(a, "arena_create (WyrmArena struct)");
    a->buf   = (char*)malloc(cap);
    wyrm_check_oom(a->buf, "arena_create (arena buffer)");
    a->cap   = cap;
    a->used  = 0;
    a->freed = 0;
    return a;
}

void *arena_alloc(WyrmArena *a, size_t sz) {
    if (!a) {
        fprintf(stderr, "Runtime Error: arena_alloc() on null arena\n");
        exit(1);
    }
    if (a->freed) {
        fprintf(stderr, "Runtime Error: arena_alloc() on destroyed arena\n");
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
    a->freed = 0;  // allow reuse after reset
}

void arena_destroy(WyrmArena *a) {
    if (!a) return;
    free(a->buf);
    a->buf   = NULL;
    a->used  = 0;
    a->freed = 1;  // mark destroyed before freeing struct
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
            wyrm_check_oom(buf, "val_split (split substring buffer)");
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
    wyrm_check_oom(strs, "val_join (temp strings array)");
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        strs[i] = val_to_str_ptr(lst.as.array->data[i]);
        total_len += strlen(strs[i]);
    }
    
    size_t sep_len = strlen(sep.as.string);
    total_len += sep_len * (size_t)(count - 1);
    
    char *res = malloc(total_len + 1);
    wyrm_check_oom(res, "val_join (joined result string)");
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
    wyrm_check_oom(buf, "val_trim (trim result buffer)");
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
    wyrm_check_oom(buf, "val_upper (upper result buffer)");
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
    wyrm_check_oom(buf, "val_lower (lower result buffer)");
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
    
    // Use careful arithmetic to avoid unsigned underflow when new_len < old_len
    size_t base_len = strlen(str);
    size_t total_len;
    if (new_len >= old_len) {
        total_len = base_len + (new_len - old_len) * (size_t)count;
    } else {
        total_len = base_len - (old_len - new_len) * (size_t)count;
    }
    char *res = malloc(total_len + 1);
    wyrm_check_oom(res, "val_replace (replace result buffer)");
    
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
    wyrm_check_oom(buf, "val_from_bytes (byte construction buffer)");
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

/*
 * wyrm_ffi.c - Wyrm Foreign Function Interface Implementation
 *
 * Implements dynamic shared library loading via dlopen (POSIX) or
 * LoadLibrary (Windows) and the four runtime FFI builtins.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --------------------------------------------------------------------------
 * Platform-specific dynamic loading
 * -------------------------------------------------------------------------- */
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

typedef HMODULE WyrmDlHandle;

static WyrmDlHandle wyrm_dl_open(const char *path) {
    return LoadLibraryA(path);
}

static void *wyrm_dl_sym(WyrmDlHandle h, const char *sym) {
    return (void *)(uintptr_t)GetProcAddress(h, sym);
}

static int wyrm_dl_close(WyrmDlHandle h) {
    return FreeLibrary(h) ? 0 : -1;
}

static const char *wyrm_dl_error(void) {
    static char buf[256];
    DWORD err = GetLastError();
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, 0, buf, (DWORD)sizeof(buf), NULL);
    return buf;
}

#else  /* POSIX */
#  include <dlfcn.h>

typedef void *WyrmDlHandle;

static WyrmDlHandle wyrm_dl_open(const char *path) {
    return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
}

static void *wyrm_dl_sym(WyrmDlHandle h, const char *sym) {
    return dlsym(h, sym);
}

static int wyrm_dl_close(WyrmDlHandle h) {
    return dlclose(h);
}

static const char *wyrm_dl_error(void) {
    return dlerror();
}

#endif /* _WIN32 */

/* --------------------------------------------------------------------------
 * ffi_open(path) -> VAL_RAW_PTR(WyrmFfiLib)
 * -------------------------------------------------------------------------- */
Value ffi_open(Value path_val) {
    if (path_val.type != VAL_STRING || !path_val.as.string) {
        fprintf(stderr, "Runtime Error [ffi_open]: argument must be a string path\n");
        exit(1);
    }

    const char *path = path_val.as.string;
    WyrmDlHandle handle = wyrm_dl_open(path);
    if (!handle) {
        const char *err = wyrm_dl_error();
        fprintf(stderr, "Runtime Error [ffi_open]: cannot load library '%s': %s\n",
                path, err ? err : "(unknown error)");
        exit(1);
    }

    WyrmFfiLib *lib = malloc(sizeof(WyrmFfiLib));
    if (!lib) {
        fprintf(stderr, "Runtime Error [ffi_open]: out of memory allocating WyrmFfiLib\n");
        exit(1);
    }
    lib->handle = (void *)handle;
    lib->path   = strdup(path);
    if (!lib->path) {
        free(lib);
        fprintf(stderr, "Runtime Error [ffi_open]: out of memory copying library path\n");
        exit(1);
    }

    return val_raw_ptr(lib);
}

/* --------------------------------------------------------------------------
 * ffi_sym(lib, symbol) -> VAL_RAW_PTR(WyrmExtFunc)
 * -------------------------------------------------------------------------- */
Value ffi_sym(Value lib_val, Value symbol_val) {
    if (lib_val.type != VAL_RAW_PTR || !lib_val.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [ffi_sym]: first argument must be an ffi_open handle\n");
        exit(1);
    }
    if (symbol_val.type != VAL_STRING || !symbol_val.as.string) {
        fprintf(stderr, "Runtime Error [ffi_sym]: second argument must be a symbol name string\n");
        exit(1);
    }

    WyrmFfiLib *lib    = (WyrmFfiLib *)lib_val.as.raw_ptr;
    const char  *sym   = symbol_val.as.string;
    WyrmDlHandle hdl   = (WyrmDlHandle)lib->handle;

    void *sym_ptr = wyrm_dl_sym(hdl, sym);
    if (!sym_ptr) {
        const char *err = wyrm_dl_error();
        fprintf(stderr, "Runtime Error [ffi_sym]: symbol '%s' not found in '%s': %s\n",
                sym, lib->path, err ? err : "(unknown error)");
        exit(1);
    }

    return val_raw_ptr(sym_ptr);
}

/* --------------------------------------------------------------------------
 * ffi_call(func, args_array) -> Value
 * --------------------------------------------------------------------------
 * Casts the raw pointer to a WyrmExtFunc and invokes it with the arguments
 * from the Wyrm array. The called function must follow WyrmExtFunc ABI.
 * -------------------------------------------------------------------------- */
Value ffi_call_fn(Value func_val, Value args_val) {
    if (func_val.type != VAL_RAW_PTR || !func_val.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [ffi_call]: first argument must be a symbol from ffi_sym\n");
        exit(1);
    }

    /* Resolve function pointer - must be WyrmExtFunc ABI */
    WyrmExtFunc fn = (WyrmExtFunc)func_val.as.raw_ptr;

    /* Accept null (zero-arg call) or array of arguments */
    Value *argv = NULL;
    int    argc = 0;

    if (args_val.type == VAL_ARRAY && args_val.as.array) {
        argc = args_val.as.array->size;
        argv = args_val.as.array->data;
    } else if (args_val.type != VAL_NULL) {
        fprintf(stderr, "Runtime Error [ffi_call]: second argument must be an array or null\n");
        exit(1);
    }

    return fn(argc, argv);
}

/* --------------------------------------------------------------------------
 * ffi_close(lib) -> VAL_NULL
 * -------------------------------------------------------------------------- */
Value ffi_close(Value lib_val) {
    if (lib_val.type != VAL_RAW_PTR || !lib_val.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [ffi_close]: argument must be an ffi_open handle\n");
        exit(1);
    }

    WyrmFfiLib  *lib = (WyrmFfiLib *)lib_val.as.raw_ptr;
    WyrmDlHandle hdl = (WyrmDlHandle)lib->handle;

    wyrm_dl_close(hdl);
    free(lib->path);
    free(lib);

    return val_null();
}
/*
 * wyrm_std_json.c - Wyrm Standard Library: JSON Implementation
 *
 * Recursive-descent JSON parser (RFC 8259 compliant) and encoder.
 * All parsing is done in a single pass over the input string.
 * Encodes Wyrm Values back to JSON text.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>

/* --------------------------------------------------------------------------
 * JSON Object Sentinel
 * -------------------------------------------------------------------------- */
#define JSON_OBJ_SENTINEL "__json_obj__"

/* --------------------------------------------------------------------------
 * Parser State
 * -------------------------------------------------------------------------- */
typedef struct {
    const char *src;
    size_t      pos;
    size_t      len;
    char        error[256];
} JsonParser;

/* Forward declarations */
static Value parse_value(JsonParser *p);
static Value parse_string(JsonParser *p);
static Value parse_number(JsonParser *p);
static Value parse_array(JsonParser *p);
static Value parse_object(JsonParser *p);

/* --------------------------------------------------------------------------
 * Parser utilities
 * -------------------------------------------------------------------------- */
static void skip_whitespace(JsonParser *p) {
    while (p->pos < p->len && isspace((unsigned char)p->src[p->pos])) {
        p->pos++;
    }
}

static char peek(JsonParser *p) {
    if (p->pos >= p->len) return '\0';
    return p->src[p->pos];
}

static char advance(JsonParser *p) {
    if (p->pos >= p->len) return '\0';
    return p->src[p->pos++];
}

static int expect_char(JsonParser *p, char c) {
    skip_whitespace(p);
    if (peek(p) != c) {
        snprintf(p->error, sizeof(p->error),
                 "JSON parse error at position %zu: expected '%c', got '%c'",
                 p->pos, c, peek(p));
        return 0;
    }
    p->pos++;
    return 1;
}

/* --------------------------------------------------------------------------
 * Parse a JSON string literal (handles \n, \t, \", \\, \uXXXX)
 * -------------------------------------------------------------------------- */
static Value parse_string(JsonParser *p) {
    if (!expect_char(p, '"')) return val_error(p->error);

    size_t cap = 64;
    size_t sz  = 0;
    char  *buf = malloc(cap);
    if (!buf) { snprintf(p->error, sizeof(p->error), "JSON OOM in string"); return val_error(p->error); }

    while (p->pos < p->len) {
        char c = advance(p);
        if (c == '"') {
            buf[sz] = '\0';
            Value result = val_string(buf);
            free(buf);
            return result;
        }
        if (c == '\\') {
            if (p->pos >= p->len) break;
            char esc = advance(p);
            char decoded = '\0';
            if      (esc == '"')  decoded = '"';
            else if (esc == '\\') decoded = '\\';
            else if (esc == '/')  decoded = '/';
            else if (esc == 'b')  decoded = '\b';
            else if (esc == 'f')  decoded = '\f';
            else if (esc == 'n')  decoded = '\n';
            else if (esc == 'r')  decoded = '\r';
            else if (esc == 't')  decoded = '\t';
            else if (esc == 'u') {
                /* \uXXXX -> decode as UTF-8 (basic BMP only) */
                if (p->pos + 4 > p->len) break;
                unsigned int cp = 0;
                for (int i = 0; i < 4; i++) {
                    char h = advance(p);
                    cp <<= 4;
                    if (h >= '0' && h <= '9')      cp |= (unsigned)(h - '0');
                    else if (h >= 'a' && h <= 'f') cp |= (unsigned)(h - 'a' + 10);
                    else if (h >= 'A' && h <= 'F') cp |= (unsigned)(h - 'A' + 10);
                }
                /* Encode as UTF-8 */
                if (sz + 4 >= cap) { cap *= 2; buf = realloc(buf, cap); if (!buf) break; }
                if (cp < 0x80) {
                    buf[sz++] = (char)cp;
                } else if (cp < 0x800) {
                    buf[sz++] = (char)(0xC0 | (cp >> 6));
                    buf[sz++] = (char)(0x80 | (cp & 0x3F));
                } else {
                    buf[sz++] = (char)(0xE0 | (cp >> 12));
                    buf[sz++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                    buf[sz++] = (char)(0x80 | (cp & 0x3F));
                }
                continue;
            }
            if (sz + 1 >= cap) { cap *= 2; buf = realloc(buf, cap); if (!buf) break; }
            buf[sz++] = decoded;
        } else {
            if (sz + 1 >= cap) { cap *= 2; buf = realloc(buf, cap); if (!buf) break; }
            buf[sz++] = c;
        }
    }

    free(buf);
    snprintf(p->error, sizeof(p->error), "JSON parse error: unterminated string");
    return val_error(p->error);
}

/* --------------------------------------------------------------------------
 * Parse a JSON number
 * -------------------------------------------------------------------------- */
static Value parse_number(JsonParser *p) {
    const char *start = p->src + p->pos;
    /* Consume optional sign, digits, decimal, exponent */
    if (peek(p) == '-') p->pos++;
    while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos])) p->pos++;
    if (peek(p) == '.') {
        p->pos++;
        while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos])) p->pos++;
    }
    if (peek(p) == 'e' || peek(p) == 'E') {
        p->pos++;
        if (peek(p) == '+' || peek(p) == '-') p->pos++;
        while (p->pos < p->len && isdigit((unsigned char)p->src[p->pos])) p->pos++;
    }
    char *end = NULL;
    errno = 0;
    double d = strtod(start, &end);
    if (errno != 0 || end == (char *)start) {
        snprintf(p->error, sizeof(p->error), "JSON parse error: invalid number at position %zu", p->pos);
        return val_error(p->error);
    }
    return val_number(d);
}

/* --------------------------------------------------------------------------
 * Parse a JSON array
 * -------------------------------------------------------------------------- */
static Value parse_array(JsonParser *p) {
    if (!expect_char(p, '[')) return val_error(p->error);
    skip_whitespace(p);

    Value arr = val_array_create(0);
    if (peek(p) == ']') { p->pos++; return arr; }

    while (p->pos < p->len) {
        skip_whitespace(p);
        Value elem = parse_value(p);
        if (elem.type == VAL_ERROR) { val_drop(arr); return elem; }
        arr = val_array_append(arr, elem);
        skip_whitespace(p);
        if (peek(p) == ']') { p->pos++; return arr; }
        if (!expect_char(p, ',')) { val_drop(arr); return val_error(p->error); }
    }

    val_drop(arr);
    snprintf(p->error, sizeof(p->error), "JSON parse error: unterminated array");
    return val_error(p->error);
}

/* --------------------------------------------------------------------------
 * Parse a JSON object -> tagged array [sentinel, k0, v0, k1, v1, ...]
 * -------------------------------------------------------------------------- */
static Value parse_object(JsonParser *p) {
    if (!expect_char(p, '{')) return val_error(p->error);
    skip_whitespace(p);

    /* Start with sentinel at index 0 */
    Value obj = val_array_create(1);
    Value sentinel = val_string(JSON_OBJ_SENTINEL);
    obj.as.array->data[0] = sentinel;
    obj.as.array->size = 1;

    if (peek(p) == '}') { p->pos++; return obj; }

    while (p->pos < p->len) {
        skip_whitespace(p);
        Value key = parse_string(p);
        if (key.type == VAL_ERROR) { val_drop(obj); return key; }
        if (!expect_char(p, ':')) { val_drop(key); val_drop(obj); return val_error(p->error); }
        skip_whitespace(p);
        Value val = parse_value(p);
        if (val.type == VAL_ERROR) { val_drop(key); val_drop(obj); return val; }

        obj = val_array_append(obj, key);
        obj = val_array_append(obj, val);

        skip_whitespace(p);
        if (peek(p) == '}') { p->pos++; return obj; }
        if (!expect_char(p, ',')) { val_drop(obj); return val_error(p->error); }
    }

    val_drop(obj);
    snprintf(p->error, sizeof(p->error), "JSON parse error: unterminated object");
    return val_error(p->error);
}

/* --------------------------------------------------------------------------
 * Top-level value dispatch
 * -------------------------------------------------------------------------- */
static Value parse_value(JsonParser *p) {
    skip_whitespace(p);
    char c = peek(p);

    if (c == '"') return parse_string(p);
    if (c == '[') return parse_array(p);
    if (c == '{') return parse_object(p);

    if (c == 't') {
        if (p->pos + 4 <= p->len && strncmp(p->src + p->pos, "true", 4) == 0) {
            p->pos += 4;
            return val_bool(true);
        }
    }
    if (c == 'f') {
        if (p->pos + 5 <= p->len && strncmp(p->src + p->pos, "false", 5) == 0) {
            p->pos += 5;
            return val_bool(false);
        }
    }
    if (c == 'n') {
        if (p->pos + 4 <= p->len && strncmp(p->src + p->pos, "null", 4) == 0) {
            p->pos += 4;
            return val_null();
        }
    }
    if (c == '-' || isdigit((unsigned char)c)) return parse_number(p);

    snprintf(p->error, sizeof(p->error),
             "JSON parse error at position %zu: unexpected character '%c'", p->pos, c);
    return val_error(p->error);
}

/* --------------------------------------------------------------------------
 * Public: json_parse
 * -------------------------------------------------------------------------- */
Value json_parse(Value json_str) {
    if (json_str.type != VAL_STRING || !json_str.as.string) {
        return val_error("json_parse: argument must be a string");
    }

    JsonParser p;
    p.src = json_str.as.string;
    p.pos = 0;
    p.len = strlen(p.src);
    p.error[0] = '\0';

    Value result = parse_value(&p);
    if (result.type == VAL_ERROR) return result;

    skip_whitespace(&p);
    if (p.pos < p.len) {
        val_drop(result);
        snprintf(p.error, sizeof(p.error),
                 "JSON parse error: trailing content at position %zu", p.pos);
        return val_error(p.error);
    }
    return result;
}

/* --------------------------------------------------------------------------
 * Encoder: dynamic string buffer
 * -------------------------------------------------------------------------- */
typedef struct {
    char  *buf;
    size_t sz;
    size_t cap;
} EncBuf;

static void enc_init(EncBuf *b) {
    b->cap = 256;
    b->sz  = 0;
    b->buf = malloc(b->cap);
    if (b->buf) b->buf[0] = '\0';
}

static void enc_append(EncBuf *b, const char *s, size_t n) {
    if (!b->buf) return;
    while (b->sz + n + 1 > b->cap) {
        b->cap *= 2;
        b->buf = realloc(b->buf, b->cap);
        if (!b->buf) return;
    }
    memcpy(b->buf + b->sz, s, n);
    b->sz += n;
    b->buf[b->sz] = '\0';
}

static void enc_char(EncBuf *b, char c) { enc_append(b, &c, 1); }

static void enc_str_escaped(EncBuf *b, const char *s) {
    enc_char(b, '"');
    if (s) {
        for (size_t i = 0; s[i]; i++) {
            unsigned char uc = (unsigned char)s[i];
            if      (uc == '"')  enc_append(b, "\\\"", 2);
            else if (uc == '\\') enc_append(b, "\\\\", 2);
            else if (uc == '\n') enc_append(b, "\\n", 2);
            else if (uc == '\r') enc_append(b, "\\r", 2);
            else if (uc == '\t') enc_append(b, "\\t", 2);
            else if (uc < 0x20) {
                char esc[7];
                snprintf(esc, sizeof(esc), "\\u%04x", uc);
                enc_append(b, esc, 6);
            } else {
                enc_char(b, (char)uc);
            }
        }
    }
    enc_char(b, '"');
}

static void encode_value(EncBuf *b, Value v, int pretty, int indent_size, int depth);

static void enc_indent(EncBuf *b, int indent_size, int depth) {
    for (int i = 0; i < indent_size * depth; i++) enc_char(b, ' ');
}

static void encode_value(EncBuf *b, Value v, int pretty, int indent_size, int depth) {
    switch (v.type) {
        case VAL_NULL:
            enc_append(b, "null", 4);
            break;
        case VAL_BOOL:
            enc_append(b, v.as.boolean ? "true" : "false", v.as.boolean ? 4 : 5);
            break;
        case VAL_NUMBER: {
            char num[64];
            double d = v.as.number;
            if (d == (long long)d && !isinf(d) && !isnan(d)) {
                snprintf(num, sizeof(num), "%.0f", d);
            } else if (isnan(d) || isinf(d)) {
                snprintf(num, sizeof(num), "null"); /* JSON has no NaN/Inf */
            } else {
                snprintf(num, sizeof(num), "%.17g", d);
            }
            enc_append(b, num, strlen(num));
            break;
        }
        case VAL_STRING:
            enc_str_escaped(b, v.as.string);
            break;
        case VAL_ARRAY:
            if (!v.as.array || v.as.array->size == 0) {
                enc_append(b, "[]", 2);
                break;
            }
            if (json_is_object(v)) {
                /* JSON object: sentinel, k0, v0, k1, v1, ... */
                enc_char(b, '{');
                int first = 1;
                for (int i = 1; i + 1 < v.as.array->size; i += 2) {
                    if (!first) { enc_char(b, ','); if (pretty) enc_char(b, '\n'); }
                    else if (pretty) enc_char(b, '\n');
                    first = 0;
                    if (pretty) enc_indent(b, indent_size, depth + 1);
                    enc_str_escaped(b, v.as.array->data[i].as.string);
                    enc_char(b, ':');
                    if (pretty) enc_char(b, ' ');
                    encode_value(b, v.as.array->data[i + 1], pretty, indent_size, depth + 1);
                }
                if (pretty) { enc_char(b, '\n'); enc_indent(b, indent_size, depth); }
                enc_char(b, '}');
            } else {
                /* JSON array */
                enc_char(b, '[');
                for (int i = 0; i < v.as.array->size; i++) {
                    if (i > 0) { enc_char(b, ','); if (pretty) enc_char(b, '\n'); }
                    else if (pretty) enc_char(b, '\n');
                    if (pretty) enc_indent(b, indent_size, depth + 1);
                    encode_value(b, v.as.array->data[i], pretty, indent_size, depth + 1);
                }
                if (pretty) { enc_char(b, '\n'); enc_indent(b, indent_size, depth); }
                enc_char(b, ']');
            }
            break;
        default:
            enc_append(b, "null", 4);
            break;
    }
}

/* --------------------------------------------------------------------------
 * Public: json_encode
 * -------------------------------------------------------------------------- */
Value json_encode(Value val) {
    EncBuf b;
    enc_init(&b);
    if (!b.buf) return val_error("json_encode: out of memory");
    encode_value(&b, val, 0, 0, 0);
    Value result = val_string(b.buf);
    free(b.buf);
    return result;
}

/* --------------------------------------------------------------------------
 * Public: json_pretty
 * -------------------------------------------------------------------------- */
Value json_pretty(Value val, Value indent_size) {
    int indent = 2;
    if (indent_size.type == VAL_NUMBER) {
        indent = (int)indent_size.as.number;
        if (indent < 0) indent = 0;
        if (indent > 16) indent = 16;
    }
    EncBuf b;
    enc_init(&b);
    if (!b.buf) return val_error("json_pretty: out of memory");
    encode_value(&b, val, 1, indent, 0);
    Value result = val_string(b.buf);
    free(b.buf);
    return result;
}

/* --------------------------------------------------------------------------
 * Public: json_is_object
 * -------------------------------------------------------------------------- */
int json_is_object(Value v) {
    if (v.type != VAL_ARRAY || !v.as.array || v.as.array->size < 1) return 0;
    Value first = v.as.array->data[0];
    if (first.type != VAL_STRING || !first.as.string) return 0;
    return strcmp(first.as.string, JSON_OBJ_SENTINEL) == 0;
}

/* --------------------------------------------------------------------------
 * Public: json_get
 * -------------------------------------------------------------------------- */
Value json_get(Value obj, Value key) {
    if (!json_is_object(obj)) return val_error("json_get: not a JSON object");
    if (key.type != VAL_STRING || !key.as.string) return val_error("json_get: key must be a string");
    for (int i = 1; i + 1 < obj.as.array->size; i += 2) {
        Value k = obj.as.array->data[i];
        if (k.type == VAL_STRING && k.as.string && strcmp(k.as.string, key.as.string) == 0) {
            return val_copy(obj.as.array->data[i + 1]);
        }
    }
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: json_has
 * -------------------------------------------------------------------------- */
Value json_has(Value obj, Value key) {
    if (!json_is_object(obj)) return val_bool(false);
    if (key.type != VAL_STRING || !key.as.string) return val_bool(false);
    for (int i = 1; i + 1 < obj.as.array->size; i += 2) {
        Value k = obj.as.array->data[i];
        if (k.type == VAL_STRING && k.as.string && strcmp(k.as.string, key.as.string) == 0) {
            return val_bool(true);
        }
    }
    return val_bool(false);
}

/* --------------------------------------------------------------------------
 * Public: json_set
 * -------------------------------------------------------------------------- */
Value json_set(Value obj, Value key, Value val) {
    if (!json_is_object(obj)) return val_error("json_set: not a JSON object");
    if (key.type != VAL_STRING || !key.as.string) return val_error("json_set: key must be a string");

    /* Update existing key */
    for (int i = 1; i + 1 < obj.as.array->size; i += 2) {
        Value k = obj.as.array->data[i];
        if (k.type == VAL_STRING && k.as.string && strcmp(k.as.string, key.as.string) == 0) {
            val_drop(obj.as.array->data[i + 1]);
            obj.as.array->data[i + 1] = val_copy(val);
            return obj;
        }
    }
    /* Append new key-value pair */
    obj = val_array_append(obj, val_copy(key));
    obj = val_array_append(obj, val_copy(val));
    return obj;
}

/* --------------------------------------------------------------------------
 * Public: json_object
 * -------------------------------------------------------------------------- */
Value json_object(void) {
    Value obj = val_array_create(1);
    Value sentinel = val_string(JSON_OBJ_SENTINEL);
    val_drop(obj.as.array->data[0]);
    obj.as.array->data[0] = sentinel;
    obj.as.array->size = 1;
    return obj;
}
/*
 * wyrm_std_yaml.c - Wyrm Standard Library: YAML Implementation
 *
 * Block-style YAML parser covering:
 *   - Mappings:  key: value
 *   - Sequences: - item
 *   - Scalars:   strings, numbers, booleans, null
 *   - Quoted strings: 'single' and "double"
 *   - Comments: # comment
 *   - Multi-level nesting via indentation
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

/* --------------------------------------------------------------------------
 * Line-oriented YAML parser
 * -------------------------------------------------------------------------- */
typedef struct {
    char **lines;
    int    count;
    int    pos;       /* current line index */
    char   error[256];
} YamlParser;

/* Forward declarations */
static Value yaml_parse_node(YamlParser *p, int base_indent);
static Value yaml_parse_scalar(const char *s);

/* --------------------------------------------------------------------------
 * Split input into lines (no strdup of entire buffer - parse in-place)
 * -------------------------------------------------------------------------- */
static YamlParser *yaml_parser_create(const char *src) {
    /* Count newlines to allocate line array */
    int count = 1;
    for (const char *c = src; *c; c++) {
        if (*c == '\n') count++;
    }

    YamlParser *p = malloc(sizeof(YamlParser));
    if (!p) return NULL;
    p->lines = malloc(sizeof(char *) * (size_t)count);
    if (!p->lines) { free(p); return NULL; }
    p->count  = 0;
    p->pos    = 0;
    p->error[0] = '\0';

    /* Duplicate input and split on newlines */
    char *dup = strdup(src);
    if (!dup) { free(p->lines); free(p); return NULL; }

    char *cursor = dup;
    while (*cursor) {
        p->lines[p->count++] = cursor;
        char *nl = strchr(cursor, '\n');
        if (!nl) break;
        *nl   = '\0';
        cursor = nl + 1;
        /* Strip carriage return */
        if (cursor > dup && *(cursor - 2) == '\r') *(cursor - 2) = '\0';
    }

    /* Store dup pointer in lines[0] for freeing later (lines[0] is start of dup) */
    return p;
}

static void yaml_parser_free(YamlParser *p) {
    if (!p) return;
    if (p->count > 0) free(p->lines[0]); /* free the strdup */
    free(p->lines);
    free(p);
}

/* Count leading spaces (indentation) */
static int count_indent(const char *line) {
    int n = 0;
    while (line[n] == ' ') n++;
    return n;
}

/* Skip blank lines and comment lines; return current line or NULL at end */
static const char *current_line(YamlParser *p) {
    while (p->pos < p->count) {
        const char *ln = p->lines[p->pos];
        /* Skip blank/comment lines */
        int i = 0;
        while (ln[i] == ' ') i++;
        if (ln[i] == '\0' || ln[i] == '#') { p->pos++; continue; }
        return ln;
    }
    return NULL;
}

/* Parse a scalar value from a string token */
static Value yaml_parse_scalar(const char *s) {
    if (!s || *s == '\0') return val_null();

    /* Strip inline comment */
    char buf[1024];
    strncpy(buf, s, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *hash = strchr(buf, '#');
    if (hash && (hash == buf || *(hash - 1) == ' ')) {
        *hash = '\0';
        /* Trim trailing space */
        size_t len = strlen(buf);
        while (len > 0 && buf[len - 1] == ' ') buf[--len] = '\0';
    }

    /* Quoted string: single quotes */
    if (buf[0] == '\'' && buf[strlen(buf) - 1] == '\'') {
        size_t len = strlen(buf) - 2;
        char *inner = malloc(len + 1);
        if (!inner) return val_null();
        memcpy(inner, buf + 1, len);
        inner[len] = '\0';
        Value v = val_string(inner);
        free(inner);
        return v;
    }

    /* Quoted string: double quotes */
    if (buf[0] == '"' && buf[strlen(buf) - 1] == '"') {
        size_t len = strlen(buf) - 2;
        char *inner = malloc(len + 1);
        if (!inner) return val_null();
        memcpy(inner, buf + 1, len);
        inner[len] = '\0';
        /* Process simple \n, \t escapes */
        char *out = inner;
        for (const char *in = inner; *in; ) {
            if (*in == '\\' && *(in + 1)) {
                in++;
                if (*in == 'n') { *out++ = '\n'; in++; }
                else if (*in == 't') { *out++ = '\t'; in++; }
                else { *out++ = *in++; }
            } else {
                *out++ = *in++;
            }
        }
        *out = '\0';
        Value v = val_string(inner);
        free(inner);
        return v;
    }

    /* Boolean */
    if (strcmp(buf, "true") == 0 || strcmp(buf, "True") == 0 || strcmp(buf, "TRUE") == 0)
        return val_bool(true);
    if (strcmp(buf, "false") == 0 || strcmp(buf, "False") == 0 || strcmp(buf, "FALSE") == 0)
        return val_bool(false);

    /* Null */
    if (strcmp(buf, "null") == 0 || strcmp(buf, "Null") == 0 ||
        strcmp(buf, "NULL") == 0 || strcmp(buf, "~") == 0 || buf[0] == '\0')
        return val_null();

    /* Number */
    char *end = NULL;
    errno = 0;
    double d = strtod(buf, &end);
    if (errno == 0 && end != buf && *end == '\0') {
        return val_number(d);
    }

    /* Plain string */
    return val_string(buf);
}

/* --------------------------------------------------------------------------
 * Recursive node parser
 * -------------------------------------------------------------------------- */
static Value yaml_parse_node(YamlParser *p, int base_indent) {
    const char *ln = current_line(p);
    if (!ln) return val_null();

    int this_indent = count_indent(ln);
    if (this_indent < base_indent) return val_null(); /* de-indented - caller handles */

    /* Detect sequence block: line starts with "- " at this_indent */
    if (ln[this_indent] == '-' && (ln[this_indent + 1] == ' ' || ln[this_indent + 1] == '\0')) {
        Value arr = val_array_create(0);
        while ((ln = current_line(p)) != NULL) {
            int ind = count_indent(ln);
            if (ind < this_indent) break;           /* de-indented: end of sequence */
            if (ind != this_indent) break;
            if (ln[ind] != '-') break;

            p->pos++; /* consume this "- " line */
            const char *item_str = ln + ind + 1;
            while (*item_str == ' ') item_str++;

            if (*item_str == '\0' || *item_str == '#') {
                /* Inline value is empty - next indented block is the value */
                Value child = yaml_parse_node(p, this_indent + 2);
                arr = val_array_append(arr, child);
            } else {
                /* Inline scalar value */
                Value item = yaml_parse_scalar(item_str);
                arr = val_array_append(arr, item);
            }
        }
        return arr;
    }

    /* Detect mapping: line contains ": " or ends with ":" */
    const char *colon = strchr(ln + this_indent, ':');
    if (colon && (colon[1] == ' ' || colon[1] == '\0' || colon[1] == '\n')) {
        Value obj = json_object();
        while ((ln = current_line(p)) != NULL) {
            int ind = count_indent(ln);
            if (ind < this_indent) break;           /* de-indented: end of mapping */
            if (ind != this_indent) break;

            colon = strchr(ln + ind, ':');
            if (!colon || (colon[1] != ' ' && colon[1] != '\0')) break;

            /* Extract key */
            size_t key_len = (size_t)(colon - (ln + ind));
            char key_buf[256];
            if (key_len >= sizeof(key_buf)) key_len = sizeof(key_buf) - 1;
            memcpy(key_buf, ln + ind, key_len);
            key_buf[key_len] = '\0';
            Value key = val_string(key_buf);

            p->pos++; /* consume key line */

            const char *value_str = colon + 1;
            while (*value_str == ' ') value_str++;

            Value val;
            if (*value_str == '\0' || *value_str == '#') {
                /* Value is on next indented block */
                val = yaml_parse_node(p, this_indent + 2);
            } else {
                val = yaml_parse_scalar(value_str);
            }

            obj = json_set(obj, key, val);
            val_drop(key);
            val_drop(val);
        }
        return obj;
    }

    /* Single scalar */
    p->pos++;
    return yaml_parse_scalar(ln + this_indent);
}

/* --------------------------------------------------------------------------
 * Public: yaml_parse
 * -------------------------------------------------------------------------- */
Value yaml_parse(Value yaml_str) {
    if (yaml_str.type != VAL_STRING || !yaml_str.as.string) {
        return val_error("yaml_parse: argument must be a string");
    }

    YamlParser *p = yaml_parser_create(yaml_str.as.string);
    if (!p) return val_error("yaml_parse: out of memory");

    /* Skip YAML document start marker "---" */
    const char *first = current_line(p);
    if (first && strncmp(first, "---", 3) == 0) p->pos++;

    Value result = yaml_parse_node(p, 0);
    yaml_parser_free(p);
    return result;
}

/* --------------------------------------------------------------------------
 * Encoder: write YAML output
 * -------------------------------------------------------------------------- */
typedef struct { char *buf; size_t sz; size_t cap; } YamlBuf;

static void yb_init(YamlBuf *b) {
    b->cap = 256; b->sz = 0; b->buf = malloc(b->cap);
    if (b->buf) b->buf[0] = '\0';
}

static void yb_append(YamlBuf *b, const char *s, size_t n) {
    if (!b->buf) return;
    while (b->sz + n + 1 > b->cap) { b->cap *= 2; b->buf = realloc(b->buf, b->cap); if (!b->buf) return; }
    memcpy(b->buf + b->sz, s, n);
    b->sz += n;
    b->buf[b->sz] = '\0';
}

static void yb_str(YamlBuf *b, const char *s) { if (s) yb_append(b, s, strlen(s)); }
static void yb_indent(YamlBuf *b, int depth) { for (int i = 0; i < depth * 2; i++) yb_append(b, " ", 1); }

static void encode_yaml(YamlBuf *b, Value v, int depth);

static void encode_yaml_scalar_str(YamlBuf *b, const char *s) {
    /* Quote strings that look like keywords or contain special characters */
    int needs_quote = 0;
    if (strcmp(s, "true") == 0 || strcmp(s, "false") == 0 ||
        strcmp(s, "null") == 0 || strcmp(s, "~") == 0) needs_quote = 1;
    if (!needs_quote) {
        for (const char *c = s; *c; c++) {
            if (*c == ':' || *c == '#' || *c == '\n' || *c == '\'') { needs_quote = 1; break; }
        }
    }
    if (needs_quote) {
        yb_append(b, "'", 1);
        for (const char *c = s; *c; c++) {
            if (*c == '\'') yb_append(b, "''", 2);
            else { char ch = *c; yb_append(b, &ch, 1); }
        }
        yb_append(b, "'", 1);
    } else {
        yb_str(b, s);
    }
}

static void encode_yaml(YamlBuf *b, Value v, int depth) {
    switch (v.type) {
        case VAL_NULL:    yb_str(b, "null"); break;
        case VAL_BOOL:    yb_str(b, v.as.boolean ? "true" : "false"); break;
        case VAL_NUMBER: {
            char num[64];
            double d = v.as.number;
            if (d == (long long)d && !isinf(d) && !isnan(d)) snprintf(num, sizeof(num), "%.0f", d);
            else if (isnan(d) || isinf(d))                    snprintf(num, sizeof(num), "null");
            else                                               snprintf(num, sizeof(num), "%.17g", d);
            yb_str(b, num);
            break;
        }
        case VAL_STRING:
            encode_yaml_scalar_str(b, v.as.string ? v.as.string : "");
            break;
        case VAL_ARRAY:
            if (!v.as.array || v.as.array->size == 0) {
                yb_str(b, json_is_object(v) ? "{}" : "[]");
                break;
            }
            if (json_is_object(v)) {
                /* Mapping */
                for (int i = 1; i + 1 < v.as.array->size; i += 2) {
                    yb_append(b, "\n", 1);
                    yb_indent(b, depth);
                    encode_yaml_scalar_str(b, v.as.array->data[i].as.string);
                    yb_str(b, ": ");
                    Value child = v.as.array->data[i + 1];
                    if (child.type == VAL_ARRAY && child.as.array && child.as.array->size > 0) {
                        encode_yaml(b, child, depth + 1);
                    } else {
                        encode_yaml(b, child, depth + 1);
                    }
                }
            } else {
                /* Sequence */
                for (int i = 0; i < v.as.array->size; i++) {
                    yb_append(b, "\n", 1);
                    yb_indent(b, depth);
                    yb_str(b, "- ");
                    Value elem = v.as.array->data[i];
                    if (elem.type == VAL_ARRAY && elem.as.array && elem.as.array->size > 0) {
                        encode_yaml(b, elem, depth + 1);
                    } else {
                        encode_yaml(b, elem, depth);
                    }
                }
            }
            break;
        default:
            yb_str(b, "null");
            break;
    }
}

/* --------------------------------------------------------------------------
 * Public: yaml_encode
 * -------------------------------------------------------------------------- */
Value yaml_encode(Value val) {
    YamlBuf b;
    yb_init(&b);
    if (!b.buf) return val_error("yaml_encode: out of memory");
    yb_str(&b, "---");
    encode_yaml(&b, val, 0);
    yb_append(&b, "\n", 1);
    Value result = val_string(b.buf);
    free(b.buf);
    return result;
}
/*
 * wyrm_std_sdl.c - Wyrm Standard Library: SDL2 Runtime Binding
 *
 * Loads SDL2 at runtime via dlopen/LoadLibrary so no compile-time SDL2
 * headers or libraries are needed. Function pointers are resolved from
 * the shared library and called through the function pointer table.
 *
 * SDL2 ABI types are redefined here to avoid the SDL2 header dependency.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* --------------------------------------------------------------------------
 * Event type constants
 * -------------------------------------------------------------------------- */
const char *const WYRM_SDL_EVT_QUIT        = "quit";
const char *const WYRM_SDL_EVT_KEYDOWN     = "keydown";
const char *const WYRM_SDL_EVT_KEYUP       = "keyup";
const char *const WYRM_SDL_EVT_MOUSEMOTION = "mousemotion";
const char *const WYRM_SDL_EVT_MOUSEDOWN   = "mousedown";
const char *const WYRM_SDL_EVT_MOUSEUP     = "mouseup";
const char *const WYRM_SDL_EVT_NONE        = "none";

/* --------------------------------------------------------------------------
 * Platform-specific dynamic loading (duplicates from wyrm_ffi.c
 * intentionally - this module is self-contained)
 * -------------------------------------------------------------------------- */
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
typedef HMODULE SdlDlHandle;
static SdlDlHandle sdl_dl_open(const char *p) { return LoadLibraryA(p); }
static void       *sdl_dl_sym(SdlDlHandle h, const char *s) { return (void *)(uintptr_t)GetProcAddress(h, s); }
#else
#  include <dlfcn.h>
typedef void *SdlDlHandle;
static SdlDlHandle sdl_dl_open(const char *p) { return dlopen(p, RTLD_LAZY | RTLD_LOCAL); }
static void       *sdl_dl_sym(SdlDlHandle h, const char *s) { return dlsym(h, s); }
#endif

/* --------------------------------------------------------------------------
 * Minimal SDL2 ABI types (redefined without SDL2 headers)
 * -------------------------------------------------------------------------- */
typedef void  SDL2_Window;
typedef void  SDL2_Renderer;
typedef uint32_t SDL2_EventType;

#define SDL2_INIT_VIDEO   0x00000020u
#define SDL2_WINDOW_SHOWN 0x00000004u

#define SDL2_QUIT          0x100u
#define SDL2_KEYDOWN       0x300u
#define SDL2_KEYUP         0x301u
#define SDL2_MOUSEMOTION   0x400u
#define SDL2_MOUSEBUTTONDOWN 0x401u
#define SDL2_MOUSEBUTTONUP   0x402u

#define SDL2_WINDOWPOS_CENTERED 0x2FFF0000u

typedef struct { uint8_t scancode; } SDL2_Keysym;
typedef struct { SDL2_EventType type; uint8_t pad[3]; SDL2_Keysym keysym; } SDL2_KeyboardEvent;
typedef struct { SDL2_EventType type; uint8_t pad[3]; int32_t x; int32_t y; int32_t xrel; int32_t yrel; } SDL2_MouseMotionEvent;
typedef struct { SDL2_EventType type; uint8_t pad[3]; uint8_t button; int32_t x; int32_t y; } SDL2_MouseButtonEvent;

typedef union {
    SDL2_EventType        type;
    SDL2_KeyboardEvent    key;
    SDL2_MouseMotionEvent motion;
    SDL2_MouseButtonEvent button;
    uint8_t               padding[56];
} SDL2_Event;

/* --------------------------------------------------------------------------
 * SDL2 function pointer table
 * -------------------------------------------------------------------------- */
typedef struct {
    SdlDlHandle handle;
    int  (*SDL_Init)(uint32_t flags);
    void (*SDL_Quit)(void);
    SDL2_Window    *(*SDL_CreateWindow)(const char *title, int x, int y, int w, int h, uint32_t flags);
    SDL2_Renderer  *(*SDL_CreateRenderer)(SDL2_Window *win, int index, uint32_t flags);
    void (*SDL_DestroyWindow)(SDL2_Window *win);
    void (*SDL_DestroyRenderer)(SDL2_Renderer *ren);
    int  (*SDL_PollEvent)(SDL2_Event *event);
    int  (*SDL_SetRenderDrawColor)(SDL2_Renderer *ren, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    int  (*SDL_RenderClear)(SDL2_Renderer *ren);
    void (*SDL_RenderPresent)(SDL2_Renderer *ren);
    int  (*SDL_RenderFillRect)(SDL2_Renderer *ren, const void *rect);
    int  (*SDL_RenderDrawLine)(SDL2_Renderer *ren, int x1, int y1, int x2, int y2);
    void (*SDL_Delay)(uint32_t ms);
    uint32_t (*SDL_GetTicks)(void);
    const char *(*SDL_GetKeyName)(int scancode);
    const char *(*SDL_GetError)(void);
} SdlFnTable;

typedef struct {
    SDL2_Window   *window;
    SDL2_Renderer *renderer;
} WyrmSdlWindow;

static SdlFnTable g_sdl = { 0 };
static int g_sdl_loaded = 0;

/* --------------------------------------------------------------------------
 * Load SDL2 shared library
 * -------------------------------------------------------------------------- */
static int load_sdl2(void) {
    if (g_sdl_loaded) return 1;

    /* Resolve library path */
    const char *lib_path = getenv("WYRM_SDL2_PATH");
#ifdef _WIN32
    const char *default_path = "SDL2.dll";
#elif defined(__APPLE__)
    const char *default_path = "libSDL2-2.0.0.dylib";
#else
    const char *default_path = "libSDL2-2.0.so.0";
#endif
    if (!lib_path) lib_path = default_path;

    SdlDlHandle h = sdl_dl_open(lib_path);
    if (!h) {
        fprintf(stderr, "Runtime Error [sdl_init]: cannot load SDL2 library '%s'.\n"
                        "  Install SDL2 or set WYRM_SDL2_PATH to point to it.\n", lib_path);
        return 0;
    }

#define LOAD_SYM(fn) \
    *(void **)(&g_sdl.fn) = sdl_dl_sym(h, #fn); \
    if (!g_sdl.fn) { fprintf(stderr, "Runtime Error [sdl_init]: SDL2 symbol '%s' not found\n", #fn); return 0; }

    LOAD_SYM(SDL_Init)
    LOAD_SYM(SDL_Quit)
    LOAD_SYM(SDL_CreateWindow)
    LOAD_SYM(SDL_CreateRenderer)
    LOAD_SYM(SDL_DestroyWindow)
    LOAD_SYM(SDL_DestroyRenderer)
    LOAD_SYM(SDL_PollEvent)
    LOAD_SYM(SDL_SetRenderDrawColor)
    LOAD_SYM(SDL_RenderClear)
    LOAD_SYM(SDL_RenderPresent)
    LOAD_SYM(SDL_RenderFillRect)
    LOAD_SYM(SDL_RenderDrawLine)
    LOAD_SYM(SDL_Delay)
    LOAD_SYM(SDL_GetTicks)
    LOAD_SYM(SDL_GetKeyName)
    LOAD_SYM(SDL_GetError)
#undef LOAD_SYM

    g_sdl.handle  = h;
    g_sdl_loaded  = 1;
    return 1;
}

/* --------------------------------------------------------------------------
 * Helper: build an event map object
 * -------------------------------------------------------------------------- */
static Value make_event_map(const char *type, const char *key,
                             int scancode, int mx, int my, int btn) {
    Value obj = json_object();
    Value vtype  = val_string(type);
    Value vkey   = val_string(key);
    Value vsc    = val_number((double)scancode);
    Value vmx    = val_number((double)mx);
    Value vmy    = val_number((double)my);
    Value vbtn   = val_number((double)btn);
    Value k_type = val_string("type");
    Value k_key  = val_string("key");
    Value k_sc   = val_string("scancode");
    Value k_mx   = val_string("mouse_x");
    Value k_my   = val_string("mouse_y");
    Value k_btn  = val_string("button");
    obj = json_set(obj, k_type, vtype); val_drop(k_type); val_drop(vtype);
    obj = json_set(obj, k_key,  vkey);  val_drop(k_key);  val_drop(vkey);
    obj = json_set(obj, k_sc,   vsc);   val_drop(k_sc);   val_drop(vsc);
    obj = json_set(obj, k_mx,   vmx);   val_drop(k_mx);   val_drop(vmx);
    obj = json_set(obj, k_my,   vmy);   val_drop(k_my);   val_drop(vmy);
    obj = json_set(obj, k_btn,  vbtn);  val_drop(k_btn);  val_drop(vbtn);
    return obj;
}

/* --------------------------------------------------------------------------
 * Public: sdl_init
 * -------------------------------------------------------------------------- */
Value sdl_init(void) {
    if (!load_sdl2()) return val_error("sdl_init: failed to load SDL2");
    if (g_sdl.SDL_Init(SDL2_INIT_VIDEO) < 0) {
        const char *err = g_sdl.SDL_GetError ? g_sdl.SDL_GetError() : "unknown";
        char msg[512];
        snprintf(msg, sizeof(msg), "sdl_init: SDL_Init failed: %s", err);
        return val_error(msg);
    }
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_quit
 * -------------------------------------------------------------------------- */
Value sdl_quit(void) {
    if (g_sdl_loaded && g_sdl.SDL_Quit) g_sdl.SDL_Quit();
    g_sdl_loaded = 0;
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_window(title, width, height)
 * -------------------------------------------------------------------------- */
Value sdl_window(Value title, Value width, Value height) {
    if (!g_sdl_loaded) return val_error("sdl_window: call sdl_init() first");
    if (title.type != VAL_STRING) return val_error("sdl_window: title must be a string");
    if (width.type != VAL_NUMBER || height.type != VAL_NUMBER)
        return val_error("sdl_window: width and height must be numbers");

    int w = (int)width.as.number;
    int h = (int)height.as.number;
    if (w <= 0 || h <= 0) return val_error("sdl_window: width and height must be positive");

    SDL2_Window *win = g_sdl.SDL_CreateWindow(
        title.as.string,
        (int)SDL2_WINDOWPOS_CENTERED, (int)SDL2_WINDOWPOS_CENTERED,
        w, h,
        SDL2_WINDOW_SHOWN
    );
    if (!win) {
        const char *err = g_sdl.SDL_GetError ? g_sdl.SDL_GetError() : "unknown";
        char msg[512];
        snprintf(msg, sizeof(msg), "sdl_window: SDL_CreateWindow failed: %s", err);
        return val_error(msg);
    }

    SDL2_Renderer *ren = g_sdl.SDL_CreateRenderer(win, -1, 0);
    if (!ren) {
        g_sdl.SDL_DestroyWindow(win);
        const char *err = g_sdl.SDL_GetError ? g_sdl.SDL_GetError() : "unknown";
        char msg[512];
        snprintf(msg, sizeof(msg), "sdl_window: SDL_CreateRenderer failed: %s", err);
        return val_error(msg);
    }

    WyrmSdlWindow *ctx = malloc(sizeof(WyrmSdlWindow));
    if (!ctx) { g_sdl.SDL_DestroyRenderer(ren); g_sdl.SDL_DestroyWindow(win); return val_error("sdl_window: OOM"); }
    ctx->window   = win;
    ctx->renderer = ren;
    return val_raw_ptr(ctx);
}

/* --------------------------------------------------------------------------
 * Public: sdl_destroy_window
 * -------------------------------------------------------------------------- */
Value sdl_destroy_window(Value win_val) {
    if (win_val.type != VAL_RAW_PTR || !win_val.as.raw_ptr) return val_null();
    WyrmSdlWindow *ctx = (WyrmSdlWindow *)win_val.as.raw_ptr;
    if (ctx->renderer) g_sdl.SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window)   g_sdl.SDL_DestroyWindow(ctx->window);
    free(ctx);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_poll_event
 * -------------------------------------------------------------------------- */
Value sdl_poll_event(void) {
    if (!g_sdl_loaded) return val_error("sdl_poll_event: call sdl_init() first");
    SDL2_Event ev;
    if (!g_sdl.SDL_PollEvent(&ev)) {
        return make_event_map(WYRM_SDL_EVT_NONE, "", 0, 0, 0, 0);
    }
    switch (ev.type) {
        case SDL2_QUIT:
            return make_event_map(WYRM_SDL_EVT_QUIT, "", 0, 0, 0, 0);
        case SDL2_KEYDOWN: {
            int sc = (int)ev.key.keysym.scancode;
            const char *kname = g_sdl.SDL_GetKeyName ? g_sdl.SDL_GetKeyName(sc) : "";
            return make_event_map(WYRM_SDL_EVT_KEYDOWN, kname ? kname : "", sc, 0, 0, 0);
        }
        case SDL2_KEYUP: {
            int sc = (int)ev.key.keysym.scancode;
            const char *kname = g_sdl.SDL_GetKeyName ? g_sdl.SDL_GetKeyName(sc) : "";
            return make_event_map(WYRM_SDL_EVT_KEYUP, kname ? kname : "", sc, 0, 0, 0);
        }
        case SDL2_MOUSEMOTION:
            return make_event_map(WYRM_SDL_EVT_MOUSEMOTION, "", 0,
                                  (int)ev.motion.x, (int)ev.motion.y, 0);
        case SDL2_MOUSEBUTTONDOWN:
            return make_event_map(WYRM_SDL_EVT_MOUSEDOWN, "", 0,
                                  (int)ev.button.x, (int)ev.button.y, (int)ev.button.button);
        case SDL2_MOUSEBUTTONUP:
            return make_event_map(WYRM_SDL_EVT_MOUSEUP, "", 0,
                                  (int)ev.button.x, (int)ev.button.y, (int)ev.button.button);
        default:
            return make_event_map(WYRM_SDL_EVT_NONE, "", 0, 0, 0, 0);
    }
}

/* --------------------------------------------------------------------------
 * Helper: validate and extract WyrmSdlWindow from Value
 * -------------------------------------------------------------------------- */
static WyrmSdlWindow *get_ctx(const char *fn, Value win_val) {
    if (win_val.type != VAL_RAW_PTR || !win_val.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [%s]: argument must be a window handle from sdl_window()\n", fn);
        exit(1);
    }
    return (WyrmSdlWindow *)win_val.as.raw_ptr;
}

/* --------------------------------------------------------------------------
 * Public: sdl_clear(win, r, g, b)
 * -------------------------------------------------------------------------- */
Value sdl_clear(Value win_val, Value r, Value g, Value b) {
    WyrmSdlWindow *ctx = get_ctx("sdl_clear", win_val);
    g_sdl.SDL_SetRenderDrawColor(ctx->renderer,
        (uint8_t)r.as.number, (uint8_t)g.as.number,
        (uint8_t)b.as.number, 255);
    g_sdl.SDL_RenderClear(ctx->renderer);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_present(win)
 * -------------------------------------------------------------------------- */
Value sdl_present(Value win_val) {
    WyrmSdlWindow *ctx = get_ctx("sdl_present", win_val);
    g_sdl.SDL_RenderPresent(ctx->renderer);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_draw_rect(win, x, y, w, h, r, g, b)
 * -------------------------------------------------------------------------- */
Value sdl_draw_rect(Value win_val, Value x, Value y, Value w, Value h,
                    Value r, Value g, Value b) {
    WyrmSdlWindow *ctx = get_ctx("sdl_draw_rect", win_val);
    g_sdl.SDL_SetRenderDrawColor(ctx->renderer,
        (uint8_t)r.as.number, (uint8_t)g.as.number,
        (uint8_t)b.as.number, 255);
    /* SDL_Rect layout: x, y, w, h (int32) */
    int32_t rect[4] = { (int32_t)x.as.number, (int32_t)y.as.number,
                         (int32_t)w.as.number, (int32_t)h.as.number };
    g_sdl.SDL_RenderFillRect(ctx->renderer, rect);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_draw_line(win, x1, y1, x2, y2, r, g, b)
 * -------------------------------------------------------------------------- */
Value sdl_draw_line(Value win_val, Value x1, Value y1, Value x2, Value y2,
                    Value r, Value g, Value b) {
    WyrmSdlWindow *ctx = get_ctx("sdl_draw_line", win_val);
    g_sdl.SDL_SetRenderDrawColor(ctx->renderer,
        (uint8_t)r.as.number, (uint8_t)g.as.number,
        (uint8_t)b.as.number, 255);
    g_sdl.SDL_RenderDrawLine(ctx->renderer,
        (int)x1.as.number, (int)y1.as.number,
        (int)x2.as.number, (int)y2.as.number);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_delay(ms)
 * -------------------------------------------------------------------------- */
Value sdl_delay(Value ms) {
    if (!g_sdl_loaded || !g_sdl.SDL_Delay) return val_null();
    uint32_t delay_ms = ms.type == VAL_NUMBER ? (uint32_t)ms.as.number : 0;
    g_sdl.SDL_Delay(delay_ms);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Public: sdl_ticks()
 * -------------------------------------------------------------------------- */
Value sdl_ticks(void) {
    if (!g_sdl_loaded || !g_sdl.SDL_GetTicks) return val_number(0);
    return val_number((double)g_sdl.SDL_GetTicks());
}
/*
 * wyrm_std_collections.c - Wyrm Standard Library: Collections Implementation
 *
 * HashMap: open-addressing hash table with FNV-1a string hashing,
 *          load-factor 0.75, power-of-2 capacity doubling.
 * Set:     thin wrapper over WyrmMap (stores VAL_BOOL true as values).
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --------------------------------------------------------------------------
 * HashMap internals
 * -------------------------------------------------------------------------- */
typedef struct MapEntry {
    char  *key;     /* heap-allocated key string; NULL = empty slot */
    Value  val;     /* stored value (owned by map) */
    int    deleted; /* tombstone marker for open addressing */
} MapEntry;

typedef struct WyrmMap {
    MapEntry *slots;
    size_t    capacity; /* must be power of 2 */
    size_t    count;    /* number of live entries */
} WyrmMap;

/* FNV-1a 64-bit hash for string keys */
static uint64_t fnv1a(const char *s) {
    uint64_t h = 14695981039346656037ULL;
    while (*s) { h ^= (uint8_t)*s++; h *= 1099511628211ULL; }
    return h;
}

static WyrmMap *map_alloc(size_t cap) {
    WyrmMap *m = calloc(1, sizeof(WyrmMap));
    if (!m) return NULL;
    m->slots    = calloc(cap, sizeof(MapEntry));
    if (!m->slots) { free(m); return NULL; }
    m->capacity = cap;
    m->count    = 0;
    return m;
}

static void map_free_internal(WyrmMap *m) {
    for (size_t i = 0; i < m->capacity; i++) {
        if (m->slots[i].key && !m->slots[i].deleted) {
            free(m->slots[i].key);
            val_drop(m->slots[i].val);
        } else if (m->slots[i].key) {
            free(m->slots[i].key); /* tombstone: only free key */
        }
    }
    free(m->slots);
    free(m);
}

static int map_insert_internal(WyrmMap *m, const char *key, Value val);

static int map_grow(WyrmMap *m) {
    size_t   new_cap   = m->capacity * 2;
    WyrmMap *nm        = map_alloc(new_cap);
    if (!nm) return 0;
    for (size_t i = 0; i < m->capacity; i++) {
        MapEntry *e = &m->slots[i];
        if (e->key && !e->deleted) {
            map_insert_internal(nm, e->key, e->val);
            /* val ownership transferred to nm - zero out to avoid double-drop */
            e->val = val_null();
        }
    }
    /* Free old slot array (keys were transferred, vals zeroed) */
    for (size_t i = 0; i < m->capacity; i++) {
        if (m->slots[i].key) free(m->slots[i].key);
    }
    free(m->slots);
    m->slots    = nm->slots;
    m->capacity = nm->capacity;
    m->count    = nm->count;
    free(nm);
    return 1;
}

static int map_insert_internal(WyrmMap *m, const char *key, Value val) {
    if (m->count * 4 >= m->capacity * 3) { /* 75% load factor */
        if (!map_grow(m)) return 0;
    }
    uint64_t h    = fnv1a(key);
    size_t   mask = m->capacity - 1;
    size_t   idx  = (size_t)(h & mask);
    size_t   first_tomb = (size_t)-1;

    for (size_t probe = 0; probe < m->capacity; probe++) {
        MapEntry *e = &m->slots[(idx + probe) & mask];
        if (!e->key) {
            /* Empty slot */
            size_t target = (first_tomb != (size_t)-1) ? first_tomb : (idx + probe) & mask;
            m->slots[target].key     = strdup(key);
            m->slots[target].val     = val;
            m->slots[target].deleted = 0;
            m->count++;
            return 1;
        }
        if (e->deleted) {
            if (first_tomb == (size_t)-1) first_tomb = (idx + probe) & mask;
            continue;
        }
        if (strcmp(e->key, key) == 0) {
            val_drop(e->val);
            e->val = val;
            return 1;
        }
    }
    return 0;
}

static MapEntry *map_find(WyrmMap *m, const char *key) {
    uint64_t h    = fnv1a(key);
    size_t   mask = m->capacity - 1;
    size_t   idx  = (size_t)(h & mask);
    for (size_t probe = 0; probe < m->capacity; probe++) {
        MapEntry *e = &m->slots[(idx + probe) & mask];
        if (!e->key) return NULL;
        if (!e->deleted && strcmp(e->key, key) == 0) return e;
    }
    return NULL;
}

/* --------------------------------------------------------------------------
 * Public: HashMap functions
 * -------------------------------------------------------------------------- */
Value map_new(void) {
    WyrmMap *m = map_alloc(16);
    if (!m) { fprintf(stderr, "Runtime Error [map_new]: out of memory\n"); exit(1); }
    return val_raw_ptr(m);
}

static WyrmMap *get_map(const char *fn, Value v) {
    if (v.type != VAL_RAW_PTR || !v.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [%s]: argument must be a map handle from map_new()\n", fn);
        exit(1);
    }
    return (WyrmMap *)v.as.raw_ptr;
}

Value map_set(Value map_val, Value key, Value val) {
    WyrmMap *m = get_map("map_set", map_val);
    if (key.type != VAL_STRING || !key.as.string) {
        fprintf(stderr, "Runtime Error [map_set]: key must be a string\n"); exit(1);
    }
    map_insert_internal(m, key.as.string, val_copy(val));
    return map_val;
}

Value map_get(Value map_val, Value key) {
    WyrmMap *m = get_map("map_get", map_val);
    if (key.type != VAL_STRING || !key.as.string) return val_null();
    MapEntry *e = map_find(m, key.as.string);
    if (!e) return val_null();
    return val_copy(e->val);
}

Value map_has(Value map_val, Value key) {
    WyrmMap *m = get_map("map_has", map_val);
    if (key.type != VAL_STRING || !key.as.string) return val_bool(false);
    return val_bool(map_find(m, key.as.string) != NULL);
}

Value map_del(Value map_val, Value key) {
    WyrmMap *m = get_map("map_del", map_val);
    if (key.type != VAL_STRING || !key.as.string) return val_null();
    uint64_t h    = fnv1a(key.as.string);
    size_t   mask = m->capacity - 1;
    size_t   idx  = (size_t)(h & mask);
    for (size_t probe = 0; probe < m->capacity; probe++) {
        MapEntry *e = &m->slots[(idx + probe) & mask];
        if (!e->key) return val_null();
        if (!e->deleted && strcmp(e->key, key.as.string) == 0) {
            val_drop(e->val);
            e->val     = val_null();
            e->deleted = 1;
            m->count--;
            return val_null();
        }
    }
    return val_null();
}

Value map_keys(Value map_val) {
    WyrmMap *m   = get_map("map_keys", map_val);
    Value    arr = val_array_create((int)m->count);
    arr.as.array->size = 0;
    for (size_t i = 0; i < m->capacity; i++) {
        MapEntry *e = &m->slots[i];
        if (e->key && !e->deleted) {
            arr = val_array_append(arr, val_string(e->key));
        }
    }
    return arr;
}

Value map_values(Value map_val) {
    WyrmMap *m   = get_map("map_values", map_val);
    Value    arr = val_array_create((int)m->count);
    arr.as.array->size = 0;
    for (size_t i = 0; i < m->capacity; i++) {
        MapEntry *e = &m->slots[i];
        if (e->key && !e->deleted) {
            arr = val_array_append(arr, val_copy(e->val));
        }
    }
    return arr;
}

Value map_len(Value map_val) {
    WyrmMap *m = get_map("map_len", map_val);
    return val_number((double)m->count);
}

Value map_free(Value map_val) {
    if (map_val.type != VAL_RAW_PTR || !map_val.as.raw_ptr) return val_null();
    map_free_internal((WyrmMap *)map_val.as.raw_ptr);
    return val_null();
}

/* --------------------------------------------------------------------------
 * Set internals: thin wrapper over WyrmMap, stores val_bool(true)
 * -------------------------------------------------------------------------- */
typedef struct WyrmSet { WyrmMap *m; } WyrmSet;

static WyrmSet *get_set(const char *fn, Value v) {
    if (v.type != VAL_RAW_PTR || !v.as.raw_ptr) {
        fprintf(stderr, "Runtime Error [%s]: argument must be a set handle from set_new()\n", fn);
        exit(1);
    }
    return (WyrmSet *)v.as.raw_ptr;
}

static const char *val_to_set_key(Value v, char *buf, size_t bufsz) {
    switch (v.type) {
        case VAL_STRING:  return v.as.string;
        case VAL_NUMBER:  snprintf(buf, bufsz, "%.17g", v.as.number);  return buf;
        case VAL_BOOL:    return v.as.boolean ? "true" : "false";
        case VAL_NULL:    return "null";
        default:          return NULL;
    }
}

Value set_new(void) {
    WyrmSet *s = malloc(sizeof(WyrmSet));
    if (!s) { fprintf(stderr, "Runtime Error [set_new]: out of memory\n"); exit(1); }
    s->m = (WyrmMap *)map_new().as.raw_ptr;
    return val_raw_ptr(s);
}

Value set_add(Value set_val, Value item) {
    WyrmSet *s = get_set("set_add", set_val);
    char buf[64];
    const char *k = val_to_set_key(item, buf, sizeof(buf));
    if (!k) { fprintf(stderr, "Runtime Error [set_add]: unsupported item type\n"); exit(1); }
    map_insert_internal(s->m, k, val_bool(true));
    return set_val;
}

Value set_has(Value set_val, Value item) {
    WyrmSet *s = get_set("set_has", set_val);
    char buf[64];
    const char *k = val_to_set_key(item, buf, sizeof(buf));
    if (!k) return val_bool(false);
    return val_bool(map_find(s->m, k) != NULL);
}

Value set_del(Value set_val, Value item) {
    WyrmSet *s = get_set("set_del", set_val);
    char buf[64];
    const char *k = val_to_set_key(item, buf, sizeof(buf));
    if (!k) return val_null();
    Value sv = val_raw_ptr(s->m);
    Value kv = k == buf ? val_string(buf) : val_null(); /* reuse key string */
    (void)kv;
    uint64_t h    = fnv1a(k);
    size_t   mask = s->m->capacity - 1;
    size_t   idx  = (size_t)(h & mask);
    for (size_t probe = 0; probe < s->m->capacity; probe++) {
        MapEntry *e = &s->m->slots[(idx + probe) & mask];
        if (!e->key) return val_null();
        if (!e->deleted && strcmp(e->key, k) == 0) {
            val_drop(e->val); e->val = val_null(); e->deleted = 1; s->m->count--; return val_null();
        }
    }
    (void)sv;
    return val_null();
}

Value set_union_fn(Value a, Value b) {
    WyrmSet *sa = get_set("set_union", a);
    WyrmSet *sb = get_set("set_union", b);
    Value result = set_new();
    WyrmSet *sr  = (WyrmSet *)result.as.raw_ptr;
    for (size_t i = 0; i < sa->m->capacity; i++) {
        MapEntry *e = &sa->m->slots[i];
        if (e->key && !e->deleted) map_insert_internal(sr->m, e->key, val_bool(true));
    }
    for (size_t i = 0; i < sb->m->capacity; i++) {
        MapEntry *e = &sb->m->slots[i];
        if (e->key && !e->deleted) map_insert_internal(sr->m, e->key, val_bool(true));
    }
    return result;
}

Value set_intersect(Value a, Value b) {
    WyrmSet *sa = get_set("set_intersect", a);
    WyrmSet *sb = get_set("set_intersect", b);
    Value result = set_new();
    WyrmSet *sr  = (WyrmSet *)result.as.raw_ptr;
    for (size_t i = 0; i < sa->m->capacity; i++) {
        MapEntry *e = &sa->m->slots[i];
        if (e->key && !e->deleted && map_find(sb->m, e->key)) {
            map_insert_internal(sr->m, e->key, val_bool(true));
        }
    }
    return result;
}

Value set_to_array(Value set_val) {
    WyrmSet *s   = get_set("set_to_array", set_val);
    Value    arr = val_array_create((int)s->m->count);
    arr.as.array->size = 0;
    for (size_t i = 0; i < s->m->capacity; i++) {
        MapEntry *e = &s->m->slots[i];
        if (e->key && !e->deleted) arr = val_array_append(arr, val_string(e->key));
    }
    return arr;
}

Value set_free(Value set_val) {
    if (set_val.type != VAL_RAW_PTR || !set_val.as.raw_ptr) return val_null();
    WyrmSet *s = (WyrmSet *)set_val.as.raw_ptr;
    map_free_internal(s->m);
    free(s);
    return val_null();
}
/*
 * wyrm_std_random.c - Wyrm Standard Library: Random Number Generation
 *
 * Implements PRNG (Xoshiro256**), CSPRNG (OS Cryptographic API),
 * and TRNG (CPU Hardware RDRAND with OS entropy fallback).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#include <immintrin.h>
#endif
#endif

/* ==========================================================================
 * PRNG: Xoshiro256** Generator & SplitMix64 Seeder
 * ========================================================================== */

static uint64_t prng_state[4] = {
    0x853c49e6748fea9bULL,
    0xda3e39cb94b95bdbULL,
    0x9e3779b97f4a7c15ULL,
    0xbf58476d1ce4e5b9ULL
};
static bool prng_initialized = false;

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t splitmix64_next(uint64_t *x) {
    uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static void xoshiro256starstar_seed(uint64_t seed) {
    uint64_t sm_state = seed;
    prng_state[0] = splitmix64_next(&sm_state);
    prng_state[1] = splitmix64_next(&sm_state);
    prng_state[2] = splitmix64_next(&sm_state);
    prng_state[3] = splitmix64_next(&sm_state);
    /* Ensure non-zero state */
    if (prng_state[0] == 0 && prng_state[1] == 0 &&
        prng_state[2] == 0 && prng_state[3] == 0) {
        prng_state[0] = 0x853c49e6748fea9bULL;
    }
    prng_initialized = true;
}

static uint64_t xoshiro256starstar_next(void) {
    const uint64_t result = rotl(prng_state[1] * 5, 7) * 9;
    const uint64_t t = prng_state[1] << 17;

    prng_state[2] ^= prng_state[0];
    prng_state[3] ^= prng_state[1];
    prng_state[1] ^= prng_state[2];
    prng_state[0] ^= prng_state[3];

    prng_state[2] ^= t;
    prng_state[3] = rotl(prng_state[3], 45);

    return result;
}

/* ==========================================================================
 * CSPRNG: OS Cryptographic Entropy
 * ========================================================================== */

#if defined(_WIN32) || defined(_WIN64)
typedef LONG (WINAPI *pfn_BCryptGenRandom)(void*, unsigned char*, unsigned long, unsigned long);
static pfn_BCryptGenRandom g_bcrypt_gen_random = NULL;
static bool g_bcrypt_loaded = false;

static bool os_crypto_random_bytes(void *buf, size_t len) {
    if (!g_bcrypt_loaded) {
        HMODULE hBcrypt = LoadLibraryA("bcrypt.dll");
        if (hBcrypt) {
            g_bcrypt_gen_random = (pfn_BCryptGenRandom)(void*)GetProcAddress(hBcrypt, "BCryptGenRandom");
        }
        g_bcrypt_loaded = true;
    }
    if (g_bcrypt_gen_random) {
        /* BCRYPT_USE_SYSTEM_PREFERRED_RNG = 0x00000002 */
        LONG status = g_bcrypt_gen_random(NULL, (unsigned char*)buf, (unsigned long)len, 0x00000002);
        if (status >= 0) return true;
    }

    /* Fallback to Advapi32 CryptGenRandom */
    HMODULE hAdvapi = LoadLibraryA("advapi32.dll");
    if (hAdvapi) {
        typedef BOOL (WINAPI *pfn_CryptAcquireContextA)(uintptr_t*, LPCSTR, LPCSTR, DWORD, DWORD);
        typedef BOOL (WINAPI *pfn_CryptGenRandom)(uintptr_t, DWORD, BYTE*);
        typedef BOOL (WINAPI *pfn_CryptReleaseContext)(uintptr_t, DWORD);

        pfn_CryptAcquireContextA pAcquire = (pfn_CryptAcquireContextA)(void*)GetProcAddress(hAdvapi, "CryptAcquireContextA");
        pfn_CryptGenRandom pGen = (pfn_CryptGenRandom)(void*)GetProcAddress(hAdvapi, "CryptGenRandom");
        pfn_CryptReleaseContext pRel = (pfn_CryptReleaseContext)(void*)GetProcAddress(hAdvapi, "CryptReleaseContext");

        if (pAcquire && pGen && pRel) {
            uintptr_t hProv = 0;
            if (pAcquire(&hProv, NULL, NULL, 1 /* PROV_RSA_FULL */, 0xF0000000 /* CRYPT_VERIFYCONTEXT */ | 0x00000040 /* CRYPT_SILENT */)) {
                BOOL ok = pGen(hProv, (DWORD)len, (BYTE*)buf);
                pRel(hProv, 0);
                if (ok) return true;
            }
        }
    }
    return false;
}
#else
static bool os_crypto_random_bytes(void *buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, (char*)buf + total, len - total);
        if (n <= 0) {
            close(fd);
            return false;
        }
        total += (size_t)n;
    }
    close(fd);
    return true;
}
#endif

/* ==========================================================================
 * TRNG: CPU Hardware RDRAND with OS entropy fallback
 * ========================================================================== */

static inline void wyrm_cpuid(unsigned int leaf, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__i386__) && defined(__PIC__)
    __asm__ volatile("xchgl %%ebx, %1\n\t"
                     "cpuid\n\t"
                     "xchgl %%ebx, %1\n\t"
                     : "=a"(*eax), "=r"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf), "c"(0));
    #else
    __asm__ volatile("cpuid"
                     : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                     : "a"(leaf), "c"(0));
    #endif
#elif defined(_MSC_VER) && !defined(__clang__)
    int info[4];
    __cpuid(info, (int)leaf);
    *eax = (unsigned int)info[0];
    *ebx = (unsigned int)info[1];
    *ecx = (unsigned int)info[2];
    *edx = (unsigned int)info[3];
#else
    *eax = *ebx = *ecx = *edx = 0;
#endif
}

static bool check_cpu_rdrand_support(void) {
#if defined(WYRM_ARCH_X86)
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    wyrm_cpuid(1, &eax, &ebx, &ecx, &edx);
    return (ecx & (1U << 30)) != 0;
#else
    return false;
#endif
}

static bool hardware_rdrand64(uint64_t *out_val) {
#if defined(WYRM_ARCH_X86) && (defined(__x86_64__) || defined(_M_X64))
    if (!check_cpu_rdrand_support()) return false;
    #if defined(__GNUC__) || defined(__clang__)
    for (int i = 0; i < 10; i++) {
        unsigned char ok;
        uint64_t v;
        __asm__ volatile(".byte 0x48, 0x0f, 0xc7, 0xf0\n\t"
                         "setc %1"
                         : "=a"(v), "=qm"(ok)
                         :
                         : "cc");
        if (ok) {
            *out_val = v;
            return true;
        }
    }
    return false;
    #elif defined(_MSC_VER) && !defined(__clang__)
    for (int i = 0; i < 10; i++) {
        if (_rdrand64_step(out_val)) return true;
    }
    return false;
    #else
    return false;
    #endif
#else
    (void)out_val;
    return false;
#endif
}

static uint64_t trng_next_u64(void) {
    uint64_t val = 0;
    if (hardware_rdrand64(&val)) {
        return val;
    }
    /* Fallback to OS cryptographic entropy pool */
    if (os_crypto_random_bytes(&val, sizeof(val))) {
        return val;
    }
    /* Final fallback to high-resolution timestamp jitter */
    uint64_t t = (uint64_t)time(NULL);
    return splitmix64_next(&t);
}

/* ==========================================================================
 * Subsystem Initialization
 * ========================================================================== */

void wyrm_random_init(void) {
    if (prng_initialized) return;
    uint64_t seed = trng_next_u64();
    xoshiro256starstar_seed(seed);
}

/* ==========================================================================
 * PRNG Wyrm Builtin Implementations
 * ========================================================================== */

Value rand_seed(Value seed_val) {
    uint64_t seed = 0;
    if (seed_val.type == VAL_NUMBER) {
        seed = (uint64_t)seed_val.as.number;
    } else if (seed_val.type == VAL_NULL) {
        seed = trng_next_u64();
    } else {
        seed = (uint64_t)time(NULL);
    }
    xoshiro256starstar_seed(seed);
    return val_null();
}

Value rand_val(void) {
    if (!prng_initialized) wyrm_random_init();
    uint64_t raw = xoshiro256starstar_next();
    /* 53-bit precision double float in [0.0, 1.0) */
    double d = (double)(raw >> 11) * (1.0 / 9007199254740992.0);
    return val_number(d);
}

static int64_t compute_rand_int(int64_t lo, int64_t hi, uint64_t (*gen_u64)(void)) {
    if (lo > hi) {
        int64_t tmp = lo;
        lo = hi;
        hi = tmp;
    }
    uint64_t range = (uint64_t)(hi - lo + 1);
    if (range == 0) {
        return lo;
    }
    /* Rejection sampling to eliminate modulo bias */
    uint64_t threshold = (0ULL - range) % range;
    uint64_t r;
    do {
        r = gen_u64();
    } while (r < threshold);

    return lo + (int64_t)(r % range);
}

Value rand_int(Value min_val, Value max_val) {
    if (!prng_initialized) wyrm_random_init();
    int64_t lo = (min_val.type == VAL_NUMBER) ? (int64_t)min_val.as.number : 0;
    int64_t hi = (max_val.type == VAL_NUMBER) ? (int64_t)max_val.as.number : 100;
    int64_t result = compute_rand_int(lo, hi, xoshiro256starstar_next);
    return val_number((double)result);
}

Value rand_range(Value min_val, Value max_val) {
    return rand_int(min_val, max_val);
}

Value rand_choice(Value arr_val) {
    if (arr_val.type != VAL_ARRAY || !arr_val.as.array || arr_val.as.array->size == 0) {
        return val_null();
    }
    int size = arr_val.as.array->size;
    int64_t idx = compute_rand_int(0, size - 1, xoshiro256starstar_next);
    return arr_val.as.array->data[idx];
}

Value rand_shuffle(Value arr_val) {
    if (arr_val.type != VAL_ARRAY || !arr_val.as.array) {
        return arr_val;
    }
    int count = arr_val.as.array->size;
    Value result = val_array_create(count);
    if (count == 0) {
        return result;
    }
    for (int i = 0; i < count; i++) {
        result.as.array->data[i] = arr_val.as.array->data[i];
    }
    /* Fisher-Yates shuffle */
    for (int i = count - 1; i > 0; i--) {
        int64_t j = compute_rand_int(0, i, xoshiro256starstar_next);
        Value temp = result.as.array->data[i];
        result.as.array->data[i] = result.as.array->data[j];
        result.as.array->data[j] = temp;
    }
    return result;
}

/* ==========================================================================
 * CSPRNG Wyrm Builtin Implementations
 * ========================================================================== */

static uint64_t crypto_next_u64(void) {
    uint64_t val = 0;
    if (!os_crypto_random_bytes(&val, sizeof(val))) {
        val = trng_next_u64();
    }
    return val;
}

Value rand_secure(void) {
    uint64_t raw = crypto_next_u64();
    double d = (double)(raw >> 11) * (1.0 / 9007199254740992.0);
    return val_number(d);
}

Value rand_secure_int(Value min_val, Value max_val) {
    int64_t lo = (min_val.type == VAL_NUMBER) ? (int64_t)min_val.as.number : 0;
    int64_t hi = (max_val.type == VAL_NUMBER) ? (int64_t)max_val.as.number : 100;
    int64_t result = compute_rand_int(lo, hi, crypto_next_u64);
    return val_number((double)result);
}

Value rand_bytes_hex(Value count_val) {
    int n = (count_val.type == VAL_NUMBER) ? (int)count_val.as.number : 16;
    if (n <= 0) return val_string("");
    if (n > 65536) n = 65536;

    unsigned char *raw = (unsigned char*)malloc((size_t)n);
    if (!raw) return val_error("Out of memory in rand_bytes_hex");

    if (!os_crypto_random_bytes(raw, (size_t)n)) {
        for (int i = 0; i < n; i++) {
            raw[i] = (unsigned char)(trng_next_u64() & 0xFF);
        }
    }

    char *hex = (char*)malloc((size_t)n * 2 + 1);
    if (!hex) {
        free(raw);
        return val_error("Out of memory in rand_bytes_hex");
    }

    static const char hex_digits[] = "0123456789abcdef";
    for (int i = 0; i < n; i++) {
        hex[i * 2]     = hex_digits[(raw[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_digits[raw[i] & 0x0F];
    }
    hex[n * 2] = '\0';
    free(raw);

    Value v = val_string(hex);
    free(hex);
    return v;
}

/* ==========================================================================
 * TRNG Wyrm Builtin Implementations
 * ========================================================================== */

Value rand_has_trng(void) {
    return val_bool(check_cpu_rdrand_support());
}

Value rand_trng(void) {
    uint64_t raw = trng_next_u64();
    double d = (double)(raw >> 11) * (1.0 / 9007199254740992.0);
    return val_number(d);
}

Value rand_trng_int(Value min_val, Value max_val) {
    int64_t lo = (min_val.type == VAL_NUMBER) ? (int64_t)min_val.as.number : 0;
    int64_t hi = (max_val.type == VAL_NUMBER) ? (int64_t)max_val.as.number : 100;
    int64_t result = compute_rand_int(lo, hi, trng_next_u64);
    return val_number((double)result);
}

Value rand_reseed_trng(void) {
    uint64_t seed = trng_next_u64();
    xoshiro256starstar_seed(seed);
    return val_null();
}
/*
 * wyrm_std_time.c - Wyrm Standard Library: High-Resolution Time Subsystem
 *
 * Implements high-resolution monotonic timers using Win32 QueryPerformanceCounter
 * on Windows and clock_gettime(CLOCK_MONOTONIC) on POSIX platforms.
 * Provides microsecond-precision wall-clock time, thread sleeping, and strftime formatting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

/* ==========================================================================
 * Monotonic Clock Internal State
 * ========================================================================== */

#if defined(_WIN32) || defined(_WIN64)
static LARGE_INTEGER g_qpc_frequency = { 0 };
static LARGE_INTEGER g_qpc_start = { 0 };
static bool g_time_initialized = false;

static void ensure_time_init(void) {
    if (!g_time_initialized) {
        QueryPerformanceFrequency(&g_qpc_frequency);
        QueryPerformanceCounter(&g_qpc_start);
        g_time_initialized = true;
    }
}
#else
static struct timespec g_posix_start = { 0, 0 };
static bool g_time_initialized = false;

static void ensure_time_init(void) {
    if (!g_time_initialized) {
        clock_gettime(CLOCK_MONOTONIC, &g_posix_start);
        g_time_initialized = true;
    }
}
#endif

void wyrm_time_init(void) {
    ensure_time_init();
}

/* ==========================================================================
 * Wall-Clock Timestamp Implementations
 * ========================================================================== */

/* Returns fractional seconds since Unix epoch with high resolution */
Value time_now(void) {
#if defined(_WIN32) || defined(_WIN64)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    /* 100-nanosecond intervals between 1601-01-01 and 1970-01-01 */
    const uint64_t epoch_diff = 116444736000000000ULL;
    if (uli.QuadPart >= epoch_diff) {
        uint64_t unix_100ns = uli.QuadPart - epoch_diff;
        double secs = (double)unix_100ns / 10000000.0;
        return val_number(secs);
    }
    return val_number((double)time(NULL));
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        double secs = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
        return val_number(secs);
    }
    return val_number((double)time(NULL));
#endif
}

/* Returns integer Unix epoch timestamp in seconds */
Value time_unix(void) {
#if defined(_WIN32) || defined(_WIN64)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    const uint64_t epoch_diff = 116444736000000000ULL;
    if (uli.QuadPart >= epoch_diff) {
        uint64_t unix_sec = (uli.QuadPart - epoch_diff) / 10000000ULL;
        return val_number((double)unix_sec);
    }
    return val_number((double)time(NULL));
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        return val_number((double)ts.tv_sec);
    }
    return val_number((double)time(NULL));
#endif
}

/* Returns integer Unix epoch timestamp in milliseconds */
Value time_unix_ms(void) {
#if defined(_WIN32) || defined(_WIN64)
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    const uint64_t epoch_diff = 116444736000000000ULL;
    if (uli.QuadPart >= epoch_diff) {
        uint64_t unix_ms = (uli.QuadPart - epoch_diff) / 10000ULL;
        return val_number((double)unix_ms);
    }
    return val_number((double)time(NULL) * 1000.0);
#else
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        double ms = ((double)ts.tv_sec * 1000.0) + ((double)ts.tv_nsec / 1000000.0);
        return val_number(ms);
    }
    return val_number((double)time(NULL) * 1000.0);
#endif
}

/* ==========================================================================
 * Monotonic High-Resolution Timers
 * ========================================================================== */

/* Returns monotonic elapsed seconds as double */
Value time_monotonic(void) {
    ensure_time_init();
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double elapsed = (double)(now.QuadPart - g_qpc_start.QuadPart) / (double)g_qpc_frequency.QuadPart;
    return val_number(elapsed);
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = (double)(now.tv_sec - g_posix_start.tv_sec) +
                     ((double)(now.tv_nsec - g_posix_start.tv_nsec) / 1000000000.0);
    return val_number(elapsed);
#endif
}

/* Returns monotonic elapsed milliseconds as double */
Value time_monotonic_ms(void) {
    ensure_time_init();
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double ms = ((double)(now.QuadPart - g_qpc_start.QuadPart) * 1000.0) / (double)g_qpc_frequency.QuadPart;
    return val_number(ms);
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double ms = ((double)(now.tv_sec - g_posix_start.tv_sec) * 1000.0) +
                ((double)(now.tv_nsec - g_posix_start.tv_nsec) / 1000000.0);
    return val_number(ms);
#endif
}

/* Returns monotonic elapsed nanoseconds as double */
Value time_monotonic_ns(void) {
    ensure_time_init();
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double ns = ((double)(now.QuadPart - g_qpc_start.QuadPart) * 1000000000.0) / (double)g_qpc_frequency.QuadPart;
    return val_number(ns);
#else
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double ns = ((double)(now.tv_sec - g_posix_start.tv_sec) * 1000000000.0) +
                (double)(now.tv_nsec - g_posix_start.tv_nsec);
    return val_number(ns);
#endif
}

/* ==========================================================================
 * Sleep and Time Arithmetic
 * ========================================================================== */

/* Suspends current thread execution for specified milliseconds */
Value time_sleep_ms(Value ms_val) {
    double ms = 0.0;
    if (ms_val.type == VAL_NUMBER) {
        ms = ms_val.as.number;
    }
    if (ms > 0.0) {
#if defined(_WIN32) || defined(_WIN64)
        DWORD sleep_ms = (DWORD)ms;
        Sleep(sleep_ms);
#else
        struct timespec req;
        req.tv_sec = (time_t)(ms / 1000.0);
        req.tv_nsec = (long)((ms - (double)req.tv_sec * 1000.0) * 1000000.0);
        nanosleep(&req, NULL);
#endif
    }
    return val_null();
}

/* Calculates difference in seconds: (end - start) */
Value time_diff(Value start_val, Value end_val) {
    double start = 0.0;
    double end = 0.0;
    if (start_val.type == VAL_NUMBER) { start = start_val.as.number; }
    if (end_val.type == VAL_NUMBER) { end = end_val.as.number; }
    return val_number(end - start);
}

/* ==========================================================================
 * Date/Time Formatting
 * ========================================================================== */

/* Formats timestamp (seconds) into UTC string using strftime */
Value time_format_utc(Value ts_val, Value fmt_val) {
    time_t raw_time;
    if (ts_val.type == VAL_NUMBER) {
        raw_time = (time_t)ts_val.as.number;
    } else {
        raw_time = time(NULL);
    }

    const char *format = "%Y-%m-%d %H:%M:%S";
    if (fmt_val.type == VAL_STRING && fmt_val.as.string && strlen(fmt_val.as.string) > 0) {
        format = fmt_val.as.string;
    }

    struct tm tm_buf;
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_buf, &raw_time);
#else
    gmtime_r(&raw_time, &tm_buf);
#endif

    char out_buf[256];
    size_t written = strftime(out_buf, sizeof(out_buf), format, &tm_buf);
    if (written == 0) {
        out_buf[0] = '\0';
    }
    return val_string(out_buf);
}

/* Formats timestamp (seconds) into local timezone string using strftime */
Value time_format_local(Value ts_val, Value fmt_val) {
    time_t raw_time;
    if (ts_val.type == VAL_NUMBER) {
        raw_time = (time_t)ts_val.as.number;
    } else {
        raw_time = time(NULL);
    }

    const char *format = "%Y-%m-%d %H:%M:%S";
    if (fmt_val.type == VAL_STRING && fmt_val.as.string && strlen(fmt_val.as.string) > 0) {
        format = fmt_val.as.string;
    }

    struct tm tm_buf;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm_buf, &raw_time);
#else
    localtime_r(&raw_time, &tm_buf);
#endif

    char out_buf[256];
    size_t written = strftime(out_buf, sizeof(out_buf), format, &tm_buf);
    if (written == 0) {
        out_buf[0] = '\0';
    }
    return val_string(out_buf);
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
Value wyrm_fn_report_diagnostic(Value wyrm_var_err_code, Value wyrm_var_title, Value wyrm_var_line_no, Value wyrm_var_col_no, Value wyrm_var_hint_msg);
Value wyrm_fn_expect(Value wyrm_var_type, Value wyrm_var_val);
Value wyrm_fn_parse_tokens(Value wyrm_var_tokens);
Value wyrm_fn_parse_statement();
Value wyrm_fn_parse_statement_inner();
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
Value wyrm_fn_has_dot(Value wyrm_var_s);
Value wyrm_fn_is_array(Value wyrm_var_v);
Value wyrm_fn_is_unboxed_type(Value wyrm_var_t);
Value wyrm_fn_canonical_type(Value wyrm_var_t);
Value wyrm_fn_get_llvm_type(Value wyrm_var_t);
Value wyrm_fn_get_llvm_align(Value wyrm_var_t);
Value wyrm_fn_can_gen_unboxed(Value wyrm_var_node, Value wyrm_var_target_t);
Value wyrm_fn_gen_unboxed_expr(Value wyrm_var_node, Value wyrm_var_target_t);
Value wyrm_fn_can_gen_static_cmp(Value wyrm_var_node);
Value wyrm_fn_gen_static_cmp(Value wyrm_var_node);
Value wyrm_fn_can_gen_i32(Value wyrm_var_node);
Value wyrm_fn_gen_i32_expr(Value wyrm_var_node);
Value wyrm_fn_can_gen_i32_cmp(Value wyrm_var_node);
Value wyrm_fn_gen_i32_cmp(Value wyrm_var_node);
Value wyrm_fn_can_gen_double(Value wyrm_var_node);
Value wyrm_fn_gen_double_expr(Value wyrm_var_node);
Value wyrm_fn_get_var_type(Value wyrm_var_name);
Value wyrm_fn_set_var_type(Value wyrm_var_name, Value wyrm_var_type);
Value wyrm_fn_infer_expr_type_in_scope(Value wyrm_var_expr);
Value wyrm_fn_scan_stmt_types(Value wyrm_var_stmt);
Value wyrm_fn_collect_globals_from_stmts(Value wyrm_var_stmts);
Value wyrm_fn_analyze_scope_types(Value wyrm_var_body, Value wyrm_var_params);
Value wyrm_fn_gen_temp();
Value wyrm_fn_emit_inst(Value wyrm_var_inst);
Value wyrm_fn_llvm_escape_string(Value wyrm_var_s);
Value wyrm_fn_push_scope();
Value wyrm_fn_pop_scope();
Value wyrm_fn_gen_all_cleanups();
Value wyrm_fn_get_llvm_str_constant(Value wyrm_var_s);
Value wyrm_fn_gen_expr(Value wyrm_var_node);
Value wyrm_fn_gen_statement(Value wyrm_var_stmt);
Value wyrm_fn_emit_label(Value wyrm_var_label);
Value wyrm_fn_check_expr(Value wyrm_var_expr, Value wyrm_var_error_funcs, Value wyrm_var_in_try_stmt, Value wyrm_var_in_try_expr, Value wyrm_var_in_catch_expr, Value wyrm_var_func_returns_error);
Value wyrm_fn_check_stmt(Value wyrm_var_stmt, Value wyrm_var_error_funcs, Value wyrm_var_in_try_stmt, Value wyrm_var_func_returns_error);
Value wyrm_fn_check_errors(Value wyrm_var_ast);
Value wyrm_fn_transpile_llvm(Value wyrm_var_ast);
Value wyrm_fn_parse_file(Value wyrm_var_path);

// Global variable declarations
Value wyrm_var_token_pos;
Value wyrm_var_g_tokens;
Value wyrm_var_current_source_file;
Value wyrm_var_current_source_lines;
Value wyrm_var_g_processed_files;
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
Value wyrm_var_emitted_globals;
Value wyrm_var_declared_locals;
Value wyrm_var_g_func_owned;
Value wyrm_var_constants;
Value wyrm_var_g_loop_breaks;
Value wyrm_var_g_loop_continues;
Value wyrm_var_current_local_types;
Value wyrm_var_global_var_types;

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
    if (val_to_bool(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_eq(wyrm_var_val, val_string("use")), val_eq(wyrm_var_val, val_string("fn"))), val_eq(wyrm_var_val, val_string("var"))), val_eq(wyrm_var_val, val_string("dec"))), val_eq(wyrm_var_val, val_string("owned"))), val_eq(wyrm_var_val, val_string("arena"))), val_eq(wyrm_var_val, val_string("if"))), val_eq(wyrm_var_val, val_string("elif"))), val_eq(wyrm_var_val, val_string("else"))), val_eq(wyrm_var_val, val_string("repeat"))), val_eq(wyrm_var_val, val_string("do"))), val_eq(wyrm_var_val, val_string("til"))), val_eq(wyrm_var_val, val_string("struct"))), val_eq(wyrm_var_val, val_string("unsafe"))), val_eq(wyrm_var_val, val_string("return"))), val_eq(wyrm_var_val, val_string("break"))), val_eq(wyrm_var_val, val_string("continue"))), val_eq(wyrm_var_val, val_string("print"))), val_eq(wyrm_var_val, val_string("and"))), val_eq(wyrm_var_val, val_string("or"))), val_eq(wyrm_var_val, val_string("not"))), val_eq(wyrm_var_val, val_string("true"))), val_eq(wyrm_var_val, val_string("false"))), val_eq(wyrm_var_val, val_string("null"))), val_eq(wyrm_var_val, val_string("try"))), val_eq(wyrm_var_val, val_string("catch"))), val_eq(wyrm_var_val, val_string("weak"))))) {
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
    wyrm_fn_report_diagnostic(val_string("E0003"), val_string("unterminated string literal"), wyrm_var_line, wyrm_var_start_col, val_string("add closing quote"));
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

Value wyrm_fn_report_diagnostic(Value wyrm_var_err_code, Value wyrm_var_title, Value wyrm_var_line_no, Value wyrm_var_col_no, Value wyrm_var_hint_msg) {
    val_print(1, val_add(val_add(val_add(val_string("error["), wyrm_var_err_code), val_string("]: ")), wyrm_var_title));
    Value wyrm_var_file_str = val_array_get(wyrm_var_current_source_file, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_file_str, val_string("")))) {
    wyrm_var_file_str = val_string("source.wyr");
    }
    val_print(1, val_add(val_add(val_add(val_add(val_add(val_string("  --> "), wyrm_var_file_str), val_string(":")), val_str(wyrm_var_line_no)), val_string(":")), val_str(wyrm_var_col_no)));
    val_print(1, val_string("   |"));
    Value wyrm_var_lines_list = val_array_get(wyrm_var_current_source_lines, val_number(0));
    if (val_to_bool(val_and(val_gt(wyrm_var_line_no, val_number(0)), val_le(wyrm_var_line_no, val_len(wyrm_var_lines_list))))) {
    Value wyrm_var_src_line = val_array_get(wyrm_var_lines_list, val_sub(wyrm_var_line_no, val_number(1)));
    Value wyrm_var_line_prefix = val_add(val_add(val_string(" "), val_str(wyrm_var_line_no)), val_string(" | "));
    val_print(1, val_add(wyrm_var_line_prefix, wyrm_var_src_line));
    Value wyrm_var_pad = val_string("   | ");
    Value wyrm_var_c = val_number(1);
    do {
    if (val_to_bool(val_ge(wyrm_var_c, wyrm_var_col_no))) {
    break;
    }
    wyrm_var_pad = val_add(wyrm_var_pad, val_string(" "));
    wyrm_var_c = val_add(wyrm_var_c, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_c, wyrm_var_col_no)));
    val_print(1, val_add(val_add(wyrm_var_pad, val_string("^ ")), wyrm_var_hint_msg));
    }
    else {
    val_print(1, val_add(val_string("   |   ^ "), wyrm_var_hint_msg));
    }
    val_print(1, val_string("   |"));
    val_exit(val_number(1));
    return val_null();
}

Value wyrm_fn_expect(Value wyrm_var_type, Value wyrm_var_val) {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_or(val_ne(val_array_get(wyrm_var_tok, val_number(0)), wyrm_var_type), val_ne(val_array_get(wyrm_var_tok, val_number(1)), wyrm_var_val)))) {
    wyrm_fn_report_diagnostic(val_string("E0001"), val_string("syntax error: unexpected token"), val_array_get(wyrm_var_tok, val_number(2)), val_array_get(wyrm_var_tok, val_number(3)), val_add(val_add(val_add(val_add(val_string("expected '"), wyrm_var_val), val_string("', got '")), val_array_get(wyrm_var_tok, val_number(1))), val_string("'")));
    }
    wyrm_fn_advance();
    return val_null();
}

Value wyrm_fn_parse_tokens(Value wyrm_var_tokens) {
    val_array_set(wyrm_var_g_tokens, val_number(0), wyrm_var_tokens);
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
    Value wyrm_var_res = wyrm_fn_parse_statement_inner();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(";")))) {
    wyrm_fn_advance();
    }
    return wyrm_var_res;
    return val_null();
}

Value wyrm_fn_parse_statement_inner() {
    Value wyrm_var_tok = wyrm_fn_current();
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")))) {
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("use")))) {
    wyrm_fn_advance();
    Value wyrm_var_path = val_string("");
    do {
    Value wyrm_var_cur_tok = wyrm_fn_current();
    if (val_to_bool(val_or(val_or(val_eq(val_array_get(wyrm_var_cur_tok, val_number(0)), val_string("EOF")), val_eq(val_array_get(wyrm_var_cur_tok, val_number(0)), val_string("NEWLINE"))), val_eq(val_array_get(wyrm_var_cur_tok, val_number(1)), val_string(";"))))) {
    break;
    }
    wyrm_var_path = val_add(wyrm_var_path, val_array_get(wyrm_var_cur_tok, val_number(1)));
    wyrm_fn_advance();
    } while (!val_to_bool(val_bool(false)));
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
    Value wyrm_var_p_name = wyrm_fn_cur_val();
    wyrm_fn_advance();
    Value wyrm_var_p_type = val_string("");
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(":")))) {
    wyrm_fn_advance();
    wyrm_var_p_type = wyrm_fn_cur_val();
    wyrm_fn_advance();
    }
    val_array_append(wyrm_var_params, wyrm_var_p_name);
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
    Value wyrm_var_ret_type = val_string("");
    if (val_to_bool(val_or(val_eq(wyrm_fn_cur_val(), val_string("->")), val_eq(wyrm_fn_cur_val(), val_string(":"))))) {
    wyrm_fn_advance();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("!")))) {
    wyrm_var_returns_error = val_bool(true);
    wyrm_fn_advance();
    }
    wyrm_var_ret_type = wyrm_fn_cur_val();
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
    return val_array_init(6, val_string("FuncDef"), wyrm_var_name, wyrm_var_params, wyrm_var_body, wyrm_var_returns_error, wyrm_var_ret_type);
    }
    if (val_to_bool(val_or(val_or(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("var")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("dec"))), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("owned"))))) {
    Value wyrm_var_type = val_array_get(wyrm_var_tok, val_number(1));
    wyrm_fn_advance();
    Value wyrm_var_name = wyrm_fn_cur_val();
    wyrm_fn_advance();
    Value wyrm_var_v_type = val_string("");
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(":")))) {
    wyrm_fn_advance();
    wyrm_var_v_type = wyrm_fn_cur_val();
    wyrm_fn_advance();
    }
    wyrm_fn_expect(val_string("OPERATOR"), val_string("="));
    Value wyrm_var_val = wyrm_fn_parse_expression();
    return val_array_init(5, val_string("VarDecl"), wyrm_var_type, wyrm_var_name, wyrm_var_val, wyrm_var_v_type);
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
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("struct")))) {
    wyrm_fn_advance();
    Value wyrm_var_name = wyrm_fn_cur_val();
    wyrm_fn_advance();
    wyrm_fn_expect(val_string("DELIMITER"), val_string("{"));
    Value wyrm_var_fields = val_array_init(0);
    Value wyrm_var_methods = val_array_init(0);
    do {
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("}")))) {
    break;
    }
    if (val_to_bool(val_eq(wyrm_fn_cur_type(), val_string("NEWLINE")))) {
    wyrm_fn_advance();
    }
    else if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("fn")))) {
    val_array_append(wyrm_var_methods, wyrm_fn_parse_statement());
    }
    else {
    Value wyrm_var_f_name = wyrm_fn_cur_val();
    wyrm_fn_advance();
    Value wyrm_var_f_type = val_string("");
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string(":")))) {
    wyrm_fn_advance();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("weak")))) {
    wyrm_fn_advance();
    wyrm_var_f_type = val_add(val_string("weak "), wyrm_fn_cur_val());
    wyrm_fn_advance();
    }
    else {
    wyrm_var_f_type = wyrm_fn_cur_val();
    wyrm_fn_advance();
    }
    }
    val_array_append(wyrm_var_fields, wyrm_var_f_name);
    if (val_to_bool(val_or(val_eq(wyrm_fn_cur_val(), val_string(",")), val_eq(wyrm_fn_cur_val(), val_string(";"))))) {
    wyrm_fn_advance();
    }
    }
    } while (!val_to_bool(val_bool(false)));
    wyrm_fn_expect(val_string("DELIMITER"), val_string("}"));
    return val_array_init(4, val_string("StructDef"), wyrm_var_name, wyrm_var_fields, wyrm_var_methods);
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
    wyrm_fn_report_diagnostic(val_string("E0010"), val_string("'repeat' loop syntax has been removed. Use 'do { ... } til <condition>' instead"), val_array_get(wyrm_var_tok, val_number(2)), val_array_get(wyrm_var_tok, val_number(3)), val_string("replace 'repeat' with 'do'"));
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("do")))) {
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
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("!")))) {
    wyrm_fn_advance();
    }
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
    if (val_to_bool(val_eq(val_array_get(wyrm_var_expr, val_number(0)), val_string("MemberAccess")))) {
    return val_array_init(4, val_string("MemberAssign"), val_array_get(wyrm_var_expr, val_number(1)), val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_val);
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
    if (val_to_bool(val_and(val_eq(val_array_get(wyrm_var_tok, val_number(0)), val_string("KEYWORD")), val_eq(val_array_get(wyrm_var_tok, val_number(1)), val_string("weak"))))) {
    wyrm_fn_advance();
    Value wyrm_var_expr = wyrm_fn_parse_unary();
    return val_array_init(2, val_string("WeakRef"), wyrm_var_expr);
    }
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
    Value wyrm_var_is_macro = val_bool(false);
    if (val_to_bool(val_and(val_eq(wyrm_fn_cur_val(), val_string("!")), val_eq(wyrm_fn_peek_val(), val_string("("))))) {
    wyrm_var_is_macro = val_bool(true);
    wyrm_fn_advance();
    }
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
    Value wyrm_var_end = val_null();
    if (val_to_bool(val_ne(wyrm_fn_cur_val(), val_string("]")))) {
    wyrm_var_end = wyrm_fn_parse_expression();
    }
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
    Value wyrm_var_member_or_method = wyrm_fn_cur_val();
    wyrm_fn_advance();
    if (val_to_bool(val_eq(wyrm_fn_cur_val(), val_string("(")))) {
    wyrm_fn_advance();
    if (val_to_bool(val_eq(wyrm_var_member_or_method, val_string("alloc")))) {
    Value wyrm_var_size = wyrm_fn_parse_expression();
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    return val_array_init(3, val_string("ArenaAlloc"), wyrm_var_name, wyrm_var_size);
    }
    if (val_to_bool(val_eq(wyrm_var_member_or_method, val_string("reset")))) {
    wyrm_fn_expect(val_string("DELIMITER"), val_string(")"));
    return val_array_init(2, val_string("ArenaReset"), wyrm_var_name);
    }
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
    return val_array_init(4, val_string("MethodCall"), val_array_init(2, val_string("Identifier"), wyrm_var_name), wyrm_var_member_or_method, wyrm_var_args);
    }
    return val_array_init(3, val_string("MemberAccess"), val_array_init(2, val_string("Identifier"), wyrm_var_name), wyrm_var_member_or_method);
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
    wyrm_fn_report_diagnostic(val_string("E0002"), val_add(val_add(val_add(val_add(val_string("syntax error: unexpected token "), val_array_get(wyrm_var_tok, val_number(0))), val_string(" '")), val_array_get(wyrm_var_tok, val_number(1))), val_string("'")), val_array_get(wyrm_var_tok, val_number(2)), val_array_get(wyrm_var_tok, val_number(3)), val_string("expression expected"));
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

Value wyrm_fn_is_array(Value wyrm_var_v) {
    if (val_to_bool(val_eq(wyrm_var_v, val_null()))) {
    return val_bool(false);
    }
    Value wyrm_var_t = val_type(wyrm_var_v);
    return val_or(val_eq(wyrm_var_t, val_string("array")), val_eq(wyrm_var_t, val_string("Array")));
    return val_null();
}

Value wyrm_fn_is_unboxed_type(Value wyrm_var_t) {
    return val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_eq(wyrm_var_t, val_string("i32")), val_eq(wyrm_var_t, val_string("int"))), val_eq(wyrm_var_t, val_string("i64"))), val_eq(wyrm_var_t, val_string("u8"))), val_eq(wyrm_var_t, val_string("f32"))), val_eq(wyrm_var_t, val_string("f64"))), val_eq(wyrm_var_t, val_string("float"))), val_eq(wyrm_var_t, val_string("Number"))), val_eq(wyrm_var_t, val_string("bool")));
    return val_null();
}

Value wyrm_fn_canonical_type(Value wyrm_var_t) {
    if (val_to_bool(val_or(val_eq(wyrm_var_t, val_string("i32")), val_eq(wyrm_var_t, val_string("int"))))) {
    return val_string("i32");
    }
    if (val_to_bool(val_eq(wyrm_var_t, val_string("i64")))) {
    return val_string("i64");
    }
    if (val_to_bool(val_eq(wyrm_var_t, val_string("u8")))) {
    return val_string("u8");
    }
    if (val_to_bool(val_eq(wyrm_var_t, val_string("f32")))) {
    return val_string("f32");
    }
    if (val_to_bool(val_or(val_or(val_eq(wyrm_var_t, val_string("f64")), val_eq(wyrm_var_t, val_string("float"))), val_eq(wyrm_var_t, val_string("Number"))))) {
    return val_string("f64");
    }
    if (val_to_bool(val_eq(wyrm_var_t, val_string("bool")))) {
    return val_string("bool");
    }
    return val_string("Dynamic");
    return val_null();
}

Value wyrm_fn_get_llvm_type(Value wyrm_var_t) {
    Value wyrm_var_c = wyrm_fn_canonical_type(wyrm_var_t);
    if (val_to_bool(val_eq(wyrm_var_c, val_string("i32")))) {
    return val_string("i32");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("i64")))) {
    return val_string("i64");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("u8")))) {
    return val_string("i8");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("f32")))) {
    return val_string("float");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("f64")))) {
    return val_string("double");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("bool")))) {
    return val_string("i1");
    }
    return val_string("%struct.Value");
    return val_null();
}

Value wyrm_fn_get_llvm_align(Value wyrm_var_t) {
    Value wyrm_var_c = wyrm_fn_canonical_type(wyrm_var_t);
    if (val_to_bool(val_eq(wyrm_var_c, val_string("i32")))) {
    return val_string("4");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("i64")))) {
    return val_string("8");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("u8")))) {
    return val_string("1");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("f32")))) {
    return val_string("4");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("f64")))) {
    return val_string("8");
    }
    if (val_to_bool(val_eq(wyrm_var_c, val_string("bool")))) {
    return val_string("1");
    }
    return val_string("8");
    return val_null();
}

Value wyrm_fn_can_gen_unboxed(Value wyrm_var_node, Value wyrm_var_target_t) {
    if (val_to_bool(val_not(wyrm_fn_is_array(wyrm_var_node)))) {
    return val_bool(false);
    }
    wyrm_var_target_t = wyrm_fn_canonical_type(wyrm_var_target_t);
    Value wyrm_var_n_type = val_array_get(wyrm_var_node, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Number")))) {
    if (val_to_bool(val_or(val_or(val_eq(wyrm_var_target_t, val_string("i32")), val_eq(wyrm_var_target_t, val_string("i64"))), val_eq(wyrm_var_target_t, val_string("u8"))))) {
    return val_not(wyrm_fn_has_dot(val_array_get(wyrm_var_node, val_number(1))));
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_target_t, val_string("f32")), val_eq(wyrm_var_target_t, val_string("f64"))))) {
    return val_bool(true);
    }
    if (val_to_bool(val_eq(wyrm_var_target_t, val_string("bool")))) {
    return val_or(val_eq(val_array_get(wyrm_var_node, val_number(1)), val_string("0")), val_eq(val_array_get(wyrm_var_node, val_number(1)), val_string("1")));
    }
    return val_bool(false);
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Boolean")))) {
    return val_eq(wyrm_var_target_t, val_string("bool"));
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Identifier")))) {
    Value wyrm_var_vt = wyrm_fn_canonical_type(wyrm_fn_get_var_type(val_array_get(wyrm_var_node, val_number(1))));
    if (val_to_bool(val_eq(wyrm_var_vt, wyrm_var_target_t))) {
    return val_bool(true);
    }
    if (val_to_bool(val_and(val_eq(wyrm_var_target_t, val_string("i64")), val_or(val_eq(wyrm_var_vt, val_string("i32")), val_eq(wyrm_var_vt, val_string("u8")))))) {
    return val_bool(true);
    }
    if (val_to_bool(val_and(val_eq(wyrm_var_target_t, val_string("i32")), val_eq(wyrm_var_vt, val_string("u8"))))) {
    return val_bool(true);
    }
    if (val_to_bool(val_and(val_eq(wyrm_var_target_t, val_string("f64")), val_or(val_or(val_eq(wyrm_var_vt, val_string("f32")), val_eq(wyrm_var_vt, val_string("i32"))), val_eq(wyrm_var_vt, val_string("u8")))))) {
    return val_bool(true);
    }
    return val_bool(false);
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("BinaryOp")))) {
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    if (val_to_bool(val_eq(wyrm_var_target_t, val_string("bool")))) {
    if (val_to_bool(val_or(val_or(val_or(val_eq(wyrm_var_op, val_string("and")), val_eq(wyrm_var_op, val_string("or"))), val_eq(wyrm_var_op, val_string("&&"))), val_eq(wyrm_var_op, val_string("||"))))) {
    return val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("bool")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("bool")));
    }
    return val_bool(false);
    }
    if (val_to_bool(val_or(val_or(val_or(val_or(val_eq(wyrm_var_op, val_string("+")), val_eq(wyrm_var_op, val_string("-"))), val_eq(wyrm_var_op, val_string("*"))), val_eq(wyrm_var_op, val_string("/"))), val_eq(wyrm_var_op, val_string("%"))))) {
    if (val_to_bool(val_and(val_or(val_eq(wyrm_var_target_t, val_string("f32")), val_eq(wyrm_var_target_t, val_string("f64"))), val_eq(wyrm_var_op, val_string("%"))))) {
    return val_bool(false);
    }
    return val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), wyrm_var_target_t), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), wyrm_var_target_t));
    }
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("UnaryOp")))) {
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    if (val_to_bool(val_and(val_eq(wyrm_var_op, val_string("-")), val_ne(wyrm_var_target_t, val_string("bool"))))) {
    return wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), wyrm_var_target_t);
    }
    if (val_to_bool(val_and(val_or(val_eq(wyrm_var_op, val_string("!")), val_eq(wyrm_var_op, val_string("not"))), val_eq(wyrm_var_target_t, val_string("bool"))))) {
    return wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("bool"));
    }
    }
    return val_bool(false);
    return val_null();
}

Value wyrm_fn_gen_unboxed_expr(Value wyrm_var_node, Value wyrm_var_target_t) {
    wyrm_var_target_t = wyrm_fn_canonical_type(wyrm_var_target_t);
    if (val_to_bool(val_eq(wyrm_var_node, val_null()))) {
    if (val_to_bool(val_or(val_eq(wyrm_var_target_t, val_string("f32")), val_eq(wyrm_var_target_t, val_string("f64"))))) {
    return val_string("0.0");
    }
    return val_string("0");
    }
    Value wyrm_var_n_type = val_array_get(wyrm_var_node, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Number")))) {
    Value wyrm_var_val = val_array_get(wyrm_var_node, val_number(1));
    if (val_to_bool(val_or(val_eq(wyrm_var_target_t, val_string("f32")), val_eq(wyrm_var_target_t, val_string("f64"))))) {
    if (val_to_bool(val_not(wyrm_fn_has_dot(wyrm_var_val)))) {
    wyrm_var_val = val_add(wyrm_var_val, val_string(".0"));
    }
    return wyrm_var_val;
    }
    return wyrm_var_val;
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Boolean")))) {
    if (val_to_bool(val_eq(val_array_get(wyrm_var_node, val_number(1)), val_string("true")))) {
    return val_string("1");
    }
    return val_string("0");
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Identifier")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_vt = wyrm_fn_canonical_type(wyrm_fn_get_var_type(wyrm_var_name));
    Value wyrm_var_reg_val = val_add(val_string("%val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    Value wyrm_var_src_llvm_t = wyrm_fn_get_llvm_type(wyrm_var_vt);
    Value wyrm_var_src_llvm_a = wyrm_fn_get_llvm_align(wyrm_var_vt);
    if (val_to_bool(val_or(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_globals, val_number(0)), wyrm_var_name), val_not(val_array_get(wyrm_var_in_function, val_number(0)))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_reg_val, val_string(" = load ")), wyrm_var_src_llvm_t), val_string(", ptr @wyrm_var_")), wyrm_var_name), val_string(", align ")), wyrm_var_src_llvm_a));
    }
    else {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_reg_val, val_string(" = load ")), wyrm_var_src_llvm_t), val_string(", ptr %wyrm_var_")), wyrm_var_name), val_string(", align ")), wyrm_var_src_llvm_a));
    }
    if (val_to_bool(val_ne(wyrm_var_vt, wyrm_var_target_t))) {
    Value wyrm_var_conv_val = val_add(val_string("%conv_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    if (val_to_bool(val_and(val_eq(wyrm_var_vt, val_string("u8")), val_eq(wyrm_var_target_t, val_string("i32"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_conv_val, val_string(" = zext i8 ")), wyrm_var_reg_val), val_string(" to i32")));
    return wyrm_var_conv_val;
    }
    if (val_to_bool(val_and(val_eq(wyrm_var_vt, val_string("u8")), val_eq(wyrm_var_target_t, val_string("i64"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_conv_val, val_string(" = zext i8 ")), wyrm_var_reg_val), val_string(" to i64")));
    return wyrm_var_conv_val;
    }
    if (val_to_bool(val_and(val_eq(wyrm_var_vt, val_string("i32")), val_eq(wyrm_var_target_t, val_string("i64"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_conv_val, val_string(" = sext i32 ")), wyrm_var_reg_val), val_string(" to i64")));
    return wyrm_var_conv_val;
    }
    if (val_to_bool(val_and(val_eq(wyrm_var_vt, val_string("f32")), val_eq(wyrm_var_target_t, val_string("f64"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_conv_val, val_string(" = fpext float ")), wyrm_var_reg_val), val_string(" to double")));
    return wyrm_var_conv_val;
    }
    if (val_to_bool(val_and(val_or(val_eq(wyrm_var_vt, val_string("i32")), val_eq(wyrm_var_vt, val_string("u8"))), val_eq(wyrm_var_target_t, val_string("f64"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(wyrm_var_conv_val, val_string(" = sitofp ")), wyrm_var_src_llvm_t), val_string(" ")), wyrm_var_reg_val), val_string(" to double")));
    return wyrm_var_conv_val;
    }
    }
    return wyrm_var_reg_val;
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("BinaryOp")))) {
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_left = wyrm_fn_gen_unboxed_expr(val_array_get(wyrm_var_node, val_number(2)), wyrm_var_target_t);
    Value wyrm_var_right = wyrm_fn_gen_unboxed_expr(val_array_get(wyrm_var_node, val_number(3)), wyrm_var_target_t);
    Value wyrm_var_res = val_add(val_string("%op_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    Value wyrm_var_llvm_t = wyrm_fn_get_llvm_type(wyrm_var_target_t);
    if (val_to_bool(val_eq(wyrm_var_target_t, val_string("bool")))) {
    if (val_to_bool(val_or(val_eq(wyrm_var_op, val_string("and")), val_eq(wyrm_var_op, val_string("&&"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = and i1 ")), wyrm_var_left), val_string(", ")), wyrm_var_right));
    return wyrm_var_res;
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_op, val_string("or")), val_eq(wyrm_var_op, val_string("||"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = or i1 ")), wyrm_var_left), val_string(", ")), wyrm_var_right));
    return wyrm_var_res;
    }
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_target_t, val_string("f32")), val_eq(wyrm_var_target_t, val_string("f64"))))) {
    Value wyrm_var_f_instr = val_string("fadd");
    if (val_to_bool(val_eq(wyrm_var_op, val_string("-")))) {
    wyrm_var_f_instr = val_string("fsub");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("*")))) {
    wyrm_var_f_instr = val_string("fmul");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("/")))) {
    wyrm_var_f_instr = val_string("fdiv");
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = ")), wyrm_var_f_instr), val_string(" ")), wyrm_var_llvm_t), val_string(" ")), wyrm_var_left), val_string(", ")), wyrm_var_right));
    return wyrm_var_res;
    }
    Value wyrm_var_i_instr = val_string("add");
    if (val_to_bool(val_eq(wyrm_var_op, val_string("-")))) {
    wyrm_var_i_instr = val_string("sub");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("*")))) {
    wyrm_var_i_instr = val_string("mul");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("/")))) {
    if (val_to_bool(val_eq(wyrm_var_target_t, val_string("u8")))) {
    wyrm_var_i_instr = val_string("udiv");
    }
    else {
    wyrm_var_i_instr = val_string("sdiv");
    }
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("%")))) {
    if (val_to_bool(val_eq(wyrm_var_target_t, val_string("u8")))) {
    wyrm_var_i_instr = val_string("urem");
    }
    else {
    wyrm_var_i_instr = val_string("srem");
    }
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = ")), wyrm_var_i_instr), val_string(" ")), wyrm_var_llvm_t), val_string(" ")), wyrm_var_left), val_string(", ")), wyrm_var_right));
    return wyrm_var_res;
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("UnaryOp")))) {
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_llvm_t = wyrm_fn_get_llvm_type(wyrm_var_target_t);
    Value wyrm_var_sub = wyrm_fn_gen_unboxed_expr(val_array_get(wyrm_var_node, val_number(2)), wyrm_var_target_t);
    Value wyrm_var_res = val_add(val_string("%un_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    if (val_to_bool(val_eq(wyrm_var_target_t, val_string("bool")))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_res, val_string(" = xor i1 ")), wyrm_var_sub), val_string(", 1")));
    return wyrm_var_res;
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_target_t, val_string("f32")), val_eq(wyrm_var_target_t, val_string("f64"))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = fneg ")), wyrm_var_llvm_t), val_string(" ")), wyrm_var_sub));
    return wyrm_var_res;
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = sub ")), wyrm_var_llvm_t), val_string(" 0, ")), wyrm_var_sub));
    return wyrm_var_res;
    }
    return val_string("0");
    return val_null();
}

Value wyrm_fn_can_gen_static_cmp(Value wyrm_var_node) {
    if (val_to_bool(val_not(wyrm_fn_is_array(wyrm_var_node)))) {
    return val_bool(false);
    }
    if (val_to_bool(val_eq(val_array_get(wyrm_var_node, val_number(0)), val_string("BinaryOp")))) {
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    if (val_to_bool(val_or(val_or(val_or(val_or(val_or(val_eq(wyrm_var_op, val_string("==")), val_eq(wyrm_var_op, val_string("!="))), val_eq(wyrm_var_op, val_string("<"))), val_eq(wyrm_var_op, val_string("<="))), val_eq(wyrm_var_op, val_string(">"))), val_eq(wyrm_var_op, val_string(">="))))) {
    if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("i32")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("i32"))))) {
    return val_bool(true);
    }
    if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("i64")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("i64"))))) {
    return val_bool(true);
    }
    if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("u8")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("u8"))))) {
    return val_bool(true);
    }
    if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("f64")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("f64"))))) {
    return val_bool(true);
    }
    if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("f32")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("f32"))))) {
    return val_bool(true);
    }
    if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("bool")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("bool"))))) {
    return val_bool(true);
    }
    }
    }
    return val_bool(false);
    return val_null();
}

Value wyrm_fn_gen_static_cmp(Value wyrm_var_node) {
    Value wyrm_var_cmp_type = val_string("i32");
    if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("i32")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("i32"))))) {
    wyrm_var_cmp_type = val_string("i32");
    }
    else if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("i64")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("i64"))))) {
    wyrm_var_cmp_type = val_string("i64");
    }
    else if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("u8")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("u8"))))) {
    wyrm_var_cmp_type = val_string("u8");
    }
    else if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("f64")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("f64"))))) {
    wyrm_var_cmp_type = val_string("f64");
    }
    else if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("f32")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("f32"))))) {
    wyrm_var_cmp_type = val_string("f32");
    }
    else if (val_to_bool(val_and(wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(2)), val_string("bool")), wyrm_fn_can_gen_unboxed(val_array_get(wyrm_var_node, val_number(3)), val_string("bool"))))) {
    wyrm_var_cmp_type = val_string("bool");
    }
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_left = wyrm_fn_gen_unboxed_expr(val_array_get(wyrm_var_node, val_number(2)), wyrm_var_cmp_type);
    Value wyrm_var_right = wyrm_fn_gen_unboxed_expr(val_array_get(wyrm_var_node, val_number(3)), wyrm_var_cmp_type);
    Value wyrm_var_res = val_add(val_string("%cmp_i1_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    Value wyrm_var_llvm_t = wyrm_fn_get_llvm_type(wyrm_var_cmp_type);
    if (val_to_bool(val_or(val_eq(wyrm_var_cmp_type, val_string("f32")), val_eq(wyrm_var_cmp_type, val_string("f64"))))) {
    Value wyrm_var_cond_op = val_string("oeq");
    if (val_to_bool(val_eq(wyrm_var_op, val_string("!=")))) {
    wyrm_var_cond_op = val_string("one");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("<")))) {
    wyrm_var_cond_op = val_string("olt");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("<=")))) {
    wyrm_var_cond_op = val_string("ole");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string(">")))) {
    wyrm_var_cond_op = val_string("ogt");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string(">=")))) {
    wyrm_var_cond_op = val_string("oge");
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = fcmp ")), wyrm_var_cond_op), val_string(" ")), wyrm_var_llvm_t), val_string(" ")), wyrm_var_left), val_string(", ")), wyrm_var_right));
    return wyrm_var_res;
    }
    if (val_to_bool(val_eq(wyrm_var_cmp_type, val_string("u8")))) {
    Value wyrm_var_cond_op = val_string("eq");
    if (val_to_bool(val_eq(wyrm_var_op, val_string("!=")))) {
    wyrm_var_cond_op = val_string("ne");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("<")))) {
    wyrm_var_cond_op = val_string("ult");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("<=")))) {
    wyrm_var_cond_op = val_string("ule");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string(">")))) {
    wyrm_var_cond_op = val_string("ugt");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string(">=")))) {
    wyrm_var_cond_op = val_string("uge");
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = icmp ")), wyrm_var_cond_op), val_string(" i8 ")), wyrm_var_left), val_string(", ")), wyrm_var_right));
    return wyrm_var_res;
    }
    Value wyrm_var_cond_op = val_string("eq");
    if (val_to_bool(val_eq(wyrm_var_op, val_string("!=")))) {
    wyrm_var_cond_op = val_string("ne");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("<")))) {
    wyrm_var_cond_op = val_string("slt");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string("<=")))) {
    wyrm_var_cond_op = val_string("sle");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string(">")))) {
    wyrm_var_cond_op = val_string("sgt");
    }
    else if (val_to_bool(val_eq(wyrm_var_op, val_string(">=")))) {
    wyrm_var_cond_op = val_string("sge");
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_res, val_string(" = icmp ")), wyrm_var_cond_op), val_string(" ")), wyrm_var_llvm_t), val_string(" ")), wyrm_var_left), val_string(", ")), wyrm_var_right));
    return wyrm_var_res;
    return val_null();
}

Value wyrm_fn_can_gen_i32(Value wyrm_var_node) {
    return wyrm_fn_can_gen_unboxed(wyrm_var_node, val_string("i32"));
    return val_null();
}

Value wyrm_fn_gen_i32_expr(Value wyrm_var_node) {
    return wyrm_fn_gen_unboxed_expr(wyrm_var_node, val_string("i32"));
    return val_null();
}

Value wyrm_fn_can_gen_i32_cmp(Value wyrm_var_node) {
    return wyrm_fn_can_gen_static_cmp(wyrm_var_node);
    return val_null();
}

Value wyrm_fn_gen_i32_cmp(Value wyrm_var_node) {
    return wyrm_fn_gen_static_cmp(wyrm_var_node);
    return val_null();
}

Value wyrm_fn_can_gen_double(Value wyrm_var_node) {
    return wyrm_fn_can_gen_unboxed(wyrm_var_node, val_string("f64"));
    return val_null();
}

Value wyrm_fn_gen_double_expr(Value wyrm_var_node) {
    return wyrm_fn_gen_unboxed_expr(wyrm_var_node, val_string("f64"));
    return val_null();
}

Value wyrm_fn_get_var_type(Value wyrm_var_name) {
    Value wyrm_var_l = val_len(wyrm_var_current_local_types);
    Value wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    Value wyrm_var_pair = val_array_get(wyrm_var_current_local_types, wyrm_var_i);
    if (val_to_bool(val_eq(val_array_get(wyrm_var_pair, val_number(0)), wyrm_var_name))) {
    return val_array_get(wyrm_var_pair, val_number(1));
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    Value wyrm_var_gl = val_len(wyrm_var_global_var_types);
    Value wyrm_var_gi = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_gi, wyrm_var_gl))) {
    break;
    }
    Value wyrm_var_gpair = val_array_get(wyrm_var_global_var_types, wyrm_var_gi);
    if (val_to_bool(val_eq(val_array_get(wyrm_var_gpair, val_number(0)), wyrm_var_name))) {
    return val_array_get(wyrm_var_gpair, val_number(1));
    }
    wyrm_var_gi = val_add(wyrm_var_gi, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_gi, wyrm_var_gl)));
    return val_string("Dynamic");
    return val_null();
}

Value wyrm_fn_set_var_type(Value wyrm_var_name, Value wyrm_var_type) {
    if (val_to_bool(val_not(val_array_get(wyrm_var_in_function, val_number(0))))) {
    Value wyrm_var_gl = val_len(wyrm_var_global_var_types);
    Value wyrm_var_gi = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_gi, wyrm_var_gl))) {
    break;
    }
    Value wyrm_var_gpair = val_array_get(wyrm_var_global_var_types, wyrm_var_gi);
    if (val_to_bool(val_eq(val_array_get(wyrm_var_gpair, val_number(0)), wyrm_var_name))) {
    val_array_set(wyrm_var_gpair, val_number(1), wyrm_var_type);
    return val_null();
    }
    wyrm_var_gi = val_add(wyrm_var_gi, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_gi, wyrm_var_gl)));
    val_array_append(wyrm_var_global_var_types, val_array_init(2, wyrm_var_name, wyrm_var_type));
    }
    Value wyrm_var_l = val_len(wyrm_var_current_local_types);
    Value wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    Value wyrm_var_pair = val_array_get(wyrm_var_current_local_types, wyrm_var_i);
    if (val_to_bool(val_eq(val_array_get(wyrm_var_pair, val_number(0)), wyrm_var_name))) {
    val_array_set(wyrm_var_pair, val_number(1), wyrm_var_type);
    return val_null();
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    val_array_append(wyrm_var_current_local_types, val_array_init(2, wyrm_var_name, wyrm_var_type));
    return val_null();
}

Value wyrm_fn_infer_expr_type_in_scope(Value wyrm_var_expr) {
    if (val_to_bool(val_not(wyrm_fn_is_array(wyrm_var_expr)))) {
    return val_string("Dynamic");
    }
    Value wyrm_var_n_type = val_array_get(wyrm_var_expr, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Number")))) {
    if (val_to_bool(val_not(wyrm_fn_has_dot(val_array_get(wyrm_var_expr, val_number(1)))))) {
    return val_string("i32");
    }
    return val_string("f64");
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Boolean")))) {
    return val_string("bool");
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Identifier")))) {
    return wyrm_fn_get_var_type(val_array_get(wyrm_var_expr, val_number(1)));
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("BinaryOp")))) {
    Value wyrm_var_op = val_array_get(wyrm_var_expr, val_number(1));
    if (val_to_bool(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_eq(wyrm_var_op, val_string("==")), val_eq(wyrm_var_op, val_string("!="))), val_eq(wyrm_var_op, val_string("<"))), val_eq(wyrm_var_op, val_string("<="))), val_eq(wyrm_var_op, val_string(">"))), val_eq(wyrm_var_op, val_string(">="))), val_eq(wyrm_var_op, val_string("and"))), val_eq(wyrm_var_op, val_string("or"))), val_eq(wyrm_var_op, val_string("&&"))), val_eq(wyrm_var_op, val_string("||"))))) {
    return val_string("bool");
    }
    if (val_to_bool(val_or(val_or(val_or(val_or(val_eq(wyrm_var_op, val_string("+")), val_eq(wyrm_var_op, val_string("-"))), val_eq(wyrm_var_op, val_string("*"))), val_eq(wyrm_var_op, val_string("/"))), val_eq(wyrm_var_op, val_string("%"))))) {
    Value wyrm_var_left_t = wyrm_fn_canonical_type(wyrm_fn_infer_expr_type_in_scope(val_array_get(wyrm_var_expr, val_number(2))));
    Value wyrm_var_right_t = wyrm_fn_canonical_type(wyrm_fn_infer_expr_type_in_scope(val_array_get(wyrm_var_expr, val_number(3))));
    if (val_to_bool(val_and(val_eq(wyrm_var_left_t, wyrm_var_right_t), wyrm_fn_is_unboxed_type(wyrm_var_left_t)))) {
    return wyrm_var_left_t;
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_left_t, val_string("f64")), val_eq(wyrm_var_right_t, val_string("f64"))))) {
    return val_string("f64");
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_left_t, val_string("f32")), val_eq(wyrm_var_right_t, val_string("f32"))))) {
    return val_string("f32");
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_left_t, val_string("i64")), val_eq(wyrm_var_right_t, val_string("i64"))))) {
    return val_string("i64");
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_left_t, val_string("i32")), val_eq(wyrm_var_right_t, val_string("i32"))))) {
    return val_string("i32");
    }
    if (val_to_bool(val_or(val_eq(wyrm_var_left_t, val_string("u8")), val_eq(wyrm_var_right_t, val_string("u8"))))) {
    return val_string("u8");
    }
    }
    return val_string("Dynamic");
    }
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("UnaryOp")))) {
    Value wyrm_var_op = val_array_get(wyrm_var_expr, val_number(1));
    if (val_to_bool(val_or(val_eq(wyrm_var_op, val_string("!")), val_eq(wyrm_var_op, val_string("not"))))) {
    return val_string("bool");
    }
    if (val_to_bool(val_eq(wyrm_var_op, val_string("-")))) {
    Value wyrm_var_sub_t = wyrm_fn_infer_expr_type_in_scope(val_array_get(wyrm_var_expr, val_number(2)));
    if (val_to_bool(wyrm_fn_is_unboxed_type(wyrm_var_sub_t))) {
    return wyrm_var_sub_t;
    }
    }
    return val_string("Dynamic");
    }
    return val_string("Dynamic");
    return val_null();
}

Value wyrm_fn_scan_stmt_types(Value wyrm_var_stmt) {
    if (val_to_bool(val_not(wyrm_fn_is_array(wyrm_var_stmt)))) {
    return val_null();
    }
    Value wyrm_var_n_type = val_array_get(wyrm_var_stmt, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_n_type, val_string("VarDecl")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_val = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_explicit_t = val_string("");
    if (val_to_bool(val_gt(val_len(wyrm_var_stmt), val_number(4)))) {
    wyrm_var_explicit_t = val_array_get(wyrm_var_stmt, val_number(4));
    }
    if (val_to_bool(wyrm_fn_is_unboxed_type(wyrm_var_explicit_t))) {
    wyrm_fn_set_var_type(wyrm_var_name, wyrm_fn_canonical_type(wyrm_var_explicit_t));
    }
    else {
    Value wyrm_var_val_type = wyrm_fn_infer_expr_type_in_scope(wyrm_var_val);
    wyrm_fn_set_var_type(wyrm_var_name, wyrm_var_val_type);
    }
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Assign")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_val = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_current_type = wyrm_fn_get_var_type(wyrm_var_name);
    if (val_to_bool(val_not(wyrm_fn_is_unboxed_type(wyrm_var_current_type)))) {
    Value wyrm_var_val_type = wyrm_fn_infer_expr_type_in_scope(wyrm_var_val);
    if (val_to_bool(val_ne(wyrm_var_current_type, wyrm_var_val_type))) {
    wyrm_fn_set_var_type(wyrm_var_name, val_string("Dynamic"));
    }
    }
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Repeat")))) {
    Value wyrm_var_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_bl = val_len(wyrm_var_body);
    Value wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_bl))) {
    break;
    }
    wyrm_fn_scan_stmt_types(val_array_get(wyrm_var_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_bl)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("If")))) {
    Value wyrm_var_then_body = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_bl = val_len(wyrm_var_then_body);
    Value wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_bl))) {
    break;
    }
    wyrm_fn_scan_stmt_types(val_array_get(wyrm_var_then_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_bl)));
    Value wyrm_var_elif_clauses = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_ecl = val_len(wyrm_var_elif_clauses);
    Value wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_ecl))) {
    break;
    }
    Value wyrm_var_clause = val_array_get(wyrm_var_elif_clauses, wyrm_var_i);
    Value wyrm_var_elif_body = val_array_get(wyrm_var_clause, val_number(1));
    Value wyrm_var_ebl = val_len(wyrm_var_elif_body);
    wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_ebl))) {
    break;
    }
    wyrm_fn_scan_stmt_types(val_array_get(wyrm_var_elif_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_ebl)));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_ecl)));
    Value wyrm_var_else_body = val_array_get(wyrm_var_stmt, val_number(4));
    Value wyrm_var_elbl = val_len(wyrm_var_else_body);
    wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_elbl))) {
    break;
    }
    wyrm_fn_scan_stmt_types(val_array_get(wyrm_var_else_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_elbl)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("TryCatch")))) {
    Value wyrm_var_try_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_tbl = val_len(wyrm_var_try_body);
    Value wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_tbl))) {
    break;
    }
    wyrm_fn_scan_stmt_types(val_array_get(wyrm_var_try_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_tbl)));
    Value wyrm_var_catch_body = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_cbl = val_len(wyrm_var_catch_body);
    wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_cbl))) {
    break;
    }
    wyrm_fn_scan_stmt_types(val_array_get(wyrm_var_catch_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_cbl)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Unsafe")))) {
    Value wyrm_var_u_body = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_ubl = val_len(wyrm_var_u_body);
    Value wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_ubl))) {
    break;
    }
    wyrm_fn_scan_stmt_types(val_array_get(wyrm_var_u_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_ubl)));
    }
    return val_null();
}

Value wyrm_fn_collect_globals_from_stmts(Value wyrm_var_stmts) {
    if (val_to_bool(val_not(wyrm_fn_is_array(wyrm_var_stmts)))) {
    return val_null();
    }
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_l = val_len(wyrm_var_stmts);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_l))) {
    break;
    }
    Value wyrm_var_stmt = val_array_get(wyrm_var_stmts, wyrm_var_i);
    if (val_to_bool(wyrm_fn_is_array(wyrm_var_stmt))) {
    Value wyrm_var_stype = val_array_get(wyrm_var_stmt, val_number(0));
    if (val_to_bool(val_eq(wyrm_var_stype, val_string("VarDecl")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(2));
    if (val_to_bool(val_not(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_globals, val_number(0)), wyrm_var_name)))) {
    val_array_append(val_array_get(wyrm_var_declared_globals, val_number(0)), wyrm_var_name);
    }
    }
    else if (val_to_bool(val_eq(wyrm_var_stype, val_string("Arena")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(1));
    if (val_to_bool(val_not(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_globals, val_number(0)), wyrm_var_name)))) {
    val_array_append(val_array_get(wyrm_var_declared_globals, val_number(0)), wyrm_var_name);
    }
    }
    else if (val_to_bool(val_eq(wyrm_var_stype, val_string("Repeat")))) {
    wyrm_fn_collect_globals_from_stmts(val_array_get(wyrm_var_stmt, val_number(1)));
    }
    else if (val_to_bool(val_eq(wyrm_var_stype, val_string("If")))) {
    wyrm_fn_collect_globals_from_stmts(val_array_get(wyrm_var_stmt, val_number(2)));
    Value wyrm_var_elif_clauses = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_el = val_len(wyrm_var_elif_clauses);
    Value wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_el))) {
    break;
    }
    Value wyrm_var_clause = val_array_get(wyrm_var_elif_clauses, wyrm_var_j);
    wyrm_fn_collect_globals_from_stmts(val_array_get(wyrm_var_clause, val_number(1)));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_el)));
    wyrm_fn_collect_globals_from_stmts(val_array_get(wyrm_var_stmt, val_number(4)));
    }
    else if (val_to_bool(val_eq(wyrm_var_stype, val_string("TryCatch")))) {
    wyrm_fn_collect_globals_from_stmts(val_array_get(wyrm_var_stmt, val_number(1)));
    wyrm_fn_collect_globals_from_stmts(val_array_get(wyrm_var_stmt, val_number(3)));
    }
    else if (val_to_bool(val_eq(wyrm_var_stype, val_string("Unsafe")))) {
    wyrm_fn_collect_globals_from_stmts(val_array_get(wyrm_var_stmt, val_number(1)));
    }
    }
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_l)));
    return val_null();
}

Value wyrm_fn_analyze_scope_types(Value wyrm_var_body, Value wyrm_var_params) {
    wyrm_var_current_local_types = val_array_init(0);
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_pl = val_len(wyrm_var_params);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_pl))) {
    break;
    }
    val_array_append(wyrm_var_current_local_types, val_array_init(2, val_array_get(wyrm_var_params, wyrm_var_i), val_string("Dynamic")));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_pl)));
    Value wyrm_var_bl = val_len(wyrm_var_body);
    wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_bl))) {
    break;
    }
    wyrm_fn_scan_stmt_types(val_array_get(wyrm_var_body, wyrm_var_i));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_bl)));
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
    val_array_append(val_array_get(wyrm_var_g_scopes, val_number(0)), val_array_init(3, val_array_init(0), val_array_init(0), val_array_init(0)));
    return val_null();
}

Value wyrm_fn_pop_scope() {
    Value wyrm_var_scopes_list = val_array_get(wyrm_var_g_scopes, val_number(0));
    Value wyrm_var_scope_idx = val_sub(val_len(wyrm_var_scopes_list), val_number(1));
    Value wyrm_var_scope = val_array_get(wyrm_var_scopes_list, wyrm_var_scope_idx);
    Value wyrm_var_arenas = val_array_get(wyrm_var_scope, val_number(0));
    Value wyrm_var_mallocs = val_array_get(wyrm_var_scope, val_number(1));
    Value wyrm_var_owned_vars = val_array_get(wyrm_var_scope, val_number(2));
    Value wyrm_var_oi = val_number(0);
    Value wyrm_var_ol = val_len(wyrm_var_owned_vars);
    do {
    if (val_to_bool(val_ge(wyrm_var_oi, wyrm_var_ol))) {
    break;
    }
    Value wyrm_var_ov = val_array_get(wyrm_var_owned_vars, wyrm_var_oi);
    Value wyrm_var_vt = wyrm_fn_canonical_type(wyrm_fn_get_var_type(wyrm_var_ov));
    if (val_to_bool(val_not(wyrm_fn_is_unboxed_type(wyrm_var_vt)))) {
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_drop(ptr %wyrm_var_"), wyrm_var_ov), val_string(")")));
    }
    wyrm_var_oi = val_add(wyrm_var_oi, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_oi, wyrm_var_ol)));
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
    Value wyrm_var_func_owned = val_array_get(wyrm_var_g_func_owned, val_number(0));
    Value wyrm_var_foi = val_number(0);
    Value wyrm_var_fol = val_len(wyrm_var_func_owned);
    do {
    if (val_to_bool(val_ge(wyrm_var_foi, wyrm_var_fol))) {
    break;
    }
    Value wyrm_var_fov = val_array_get(wyrm_var_func_owned, wyrm_var_foi);
    Value wyrm_var_vt = wyrm_fn_canonical_type(wyrm_fn_get_var_type(wyrm_var_fov));
    if (val_to_bool(val_not(wyrm_fn_is_unboxed_type(wyrm_var_vt)))) {
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_drop(ptr %wyrm_var_"), wyrm_var_fov), val_string(")")));
    }
    wyrm_var_foi = val_add(wyrm_var_foi, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_foi, wyrm_var_fol)));
    Value wyrm_var_scopes_list = val_array_get(wyrm_var_g_scopes, val_number(0));
    Value wyrm_var_i = val_sub(val_len(wyrm_var_scopes_list), val_number(1));
    do {
    if (val_to_bool(val_lt(wyrm_var_i, val_number(0)))) {
    break;
    }
    Value wyrm_var_scope = val_array_get(wyrm_var_scopes_list, wyrm_var_i);
    Value wyrm_var_arenas = val_array_get(wyrm_var_scope, val_number(0));
    Value wyrm_var_mallocs = val_array_get(wyrm_var_scope, val_number(1));
    Value wyrm_var_owned_vars = val_array_get(wyrm_var_scope, val_number(2));
    Value wyrm_var_oi = val_number(0);
    Value wyrm_var_ol = val_len(wyrm_var_owned_vars);
    do {
    if (val_to_bool(val_ge(wyrm_var_oi, wyrm_var_ol))) {
    break;
    }
    Value wyrm_var_ov = val_array_get(wyrm_var_owned_vars, wyrm_var_oi);
    Value wyrm_var_vt = wyrm_fn_canonical_type(wyrm_fn_get_var_type(wyrm_var_ov));
    if (val_to_bool(val_not(wyrm_fn_is_unboxed_type(wyrm_var_vt)))) {
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_drop(ptr %wyrm_var_"), wyrm_var_ov), val_string(")")));
    }
    wyrm_var_oi = val_add(wyrm_var_oi, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_oi, wyrm_var_ol)));
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

Value wyrm_fn_get_llvm_str_constant(Value wyrm_var_s) {
    Value wyrm_var_str_idx = val_len(wyrm_var_g_llvm_globals);
    Value wyrm_var_str_name = val_add(val_string("@.str."), val_str(wyrm_var_str_idx));
    Value wyrm_var_escaped = wyrm_fn_llvm_escape_string(wyrm_var_s);
    Value wyrm_var_str_len = val_add(val_len(wyrm_var_s), val_number(1));
    val_array_append(wyrm_var_g_llvm_globals, val_add(val_add(val_add(val_add(val_add(wyrm_var_str_name, val_string(" = private unnamed_addr constant [")), val_str(wyrm_var_str_len)), val_string(" x i8] c\"")), wyrm_var_escaped), val_string("\\00\", align 1")));
    return wyrm_var_str_name;
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
    Value wyrm_var_str_name = wyrm_fn_get_llvm_str_constant(val_array_get(wyrm_var_node, val_number(1)));
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
    Value wyrm_var_catch_vars_list = val_array_get(wyrm_var_g_catch_vars, val_number(0));
    Value wyrm_var_active_var = val_array_get(wyrm_var_catch_vars_list, val_sub(wyrm_var_num_labels, val_number(1)));
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
    Value wyrm_var_name = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_v_type = wyrm_fn_canonical_type(wyrm_fn_get_var_type(wyrm_var_name));
    if (val_to_bool(wyrm_fn_is_unboxed_type(wyrm_var_v_type))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_llvm_t = wyrm_fn_get_llvm_type(wyrm_var_v_type);
    Value wyrm_var_llvm_a = wyrm_fn_get_llvm_align(wyrm_var_v_type);
    Value wyrm_var_raw_val = val_add(val_string("%raw_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    if (val_to_bool(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_globals, val_number(0)), wyrm_var_name))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_raw_val, val_string(" = load ")), wyrm_var_llvm_t), val_string(", ptr @wyrm_var_")), wyrm_var_name), val_string(", align ")), wyrm_var_llvm_a));
    }
    else {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_raw_val, val_string(" = load ")), wyrm_var_llvm_t), val_string(", ptr %wyrm_var_")), wyrm_var_name), val_string(", align ")), wyrm_var_llvm_a));
    }
    if (val_to_bool(val_eq(wyrm_var_v_type, val_string("i32")))) {
    Value wyrm_var_dbl_conv = val_add(val_string("%dbl_conv_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_dbl_conv, val_string(" = sitofp i32 ")), wyrm_var_raw_val), val_string(" to double")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_number(ptr "), wyrm_var_t), val_string(", double ")), wyrm_var_dbl_conv), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_v_type, val_string("i64")))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_from_i64(ptr "), wyrm_var_t), val_string(", i64 ")), wyrm_var_raw_val), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_v_type, val_string("u8")))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_from_u8(ptr "), wyrm_var_t), val_string(", i8 ")), wyrm_var_raw_val), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_v_type, val_string("f32")))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_from_f32(ptr "), wyrm_var_t), val_string(", float ")), wyrm_var_raw_val), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_v_type, val_string("f64")))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_number(ptr "), wyrm_var_t), val_string(", double ")), wyrm_var_raw_val), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_v_type, val_string("bool")))) {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_from_bool(ptr "), wyrm_var_t), val_string(", i1 ")), wyrm_var_raw_val), val_string(")")));
    return wyrm_var_t;
    }
    }
    if (val_to_bool(val_or(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_globals, val_number(0)), wyrm_var_name), val_not(val_array_get(wyrm_var_in_function, val_number(0)))))) {
    return val_add(val_string("@wyrm_var_"), wyrm_var_name);
    }
    return val_add(val_string("%wyrm_var_"), wyrm_var_name);
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("BinaryOp")))) {
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    if (val_to_bool(val_or(val_or(val_or(val_or(val_eq(wyrm_var_op, val_string("+")), val_eq(wyrm_var_op, val_string("-"))), val_eq(wyrm_var_op, val_string("*"))), val_eq(wyrm_var_op, val_string("/"))), val_eq(wyrm_var_op, val_string("%"))))) {
    if (val_to_bool(wyrm_fn_can_gen_unboxed(wyrm_var_node, val_string("i32")))) {
    Value wyrm_var_i32_res = wyrm_fn_gen_unboxed_expr(wyrm_var_node, val_string("i32"));
    Value wyrm_var_dbl_conv = val_add(val_string("%dbl_conv_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_dbl_conv, val_string(" = sitofp i32 ")), wyrm_var_i32_res), val_string(" to double")));
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_number(ptr "), wyrm_var_t), val_string(", double ")), wyrm_var_dbl_conv), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(wyrm_fn_can_gen_unboxed(wyrm_var_node, val_string("i64")))) {
    Value wyrm_var_i64_res = wyrm_fn_gen_unboxed_expr(wyrm_var_node, val_string("i64"));
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_from_i64(ptr "), wyrm_var_t), val_string(", i64 ")), wyrm_var_i64_res), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(wyrm_fn_can_gen_unboxed(wyrm_var_node, val_string("u8")))) {
    Value wyrm_var_u8_res = wyrm_fn_gen_unboxed_expr(wyrm_var_node, val_string("u8"));
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_from_u8(ptr "), wyrm_var_t), val_string(", i8 ")), wyrm_var_u8_res), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(wyrm_fn_can_gen_unboxed(wyrm_var_node, val_string("f32")))) {
    Value wyrm_var_f32_res = wyrm_fn_gen_unboxed_expr(wyrm_var_node, val_string("f32"));
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_from_f32(ptr "), wyrm_var_t), val_string(", float ")), wyrm_var_f32_res), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(wyrm_fn_can_gen_unboxed(wyrm_var_node, val_string("f64")))) {
    Value wyrm_var_dbl_res = wyrm_fn_gen_unboxed_expr(wyrm_var_node, val_string("f64"));
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_number(ptr "), wyrm_var_t), val_string(", double ")), wyrm_var_dbl_res), val_string(")")));
    return wyrm_var_t;
    }
    }
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_left_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(2)));
    Value wyrm_var_right_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(3)));
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
    Value wyrm_var_op = val_array_get(wyrm_var_node, val_number(1));
    if (val_to_bool(val_and(val_eq(wyrm_var_op, val_string("-")), wyrm_fn_can_gen_double(wyrm_var_node)))) {
    Value wyrm_var_dbl_res = wyrm_fn_gen_double_expr(wyrm_var_node);
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_number(ptr "), wyrm_var_t), val_string(", double ")), wyrm_var_dbl_res), val_string(")")));
    return wyrm_var_t;
    }
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_expr_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(2)));
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
    if (val_to_bool(val_eq(wyrm_var_type, val_string("WeakRef")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_target_reg = wyrm_fn_gen_expr(val_array_get(wyrm_var_node, val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_weak_ref(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_target_reg), val_string(")")));
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
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_arena_alloc(ptr "), wyrm_var_t), val_string(", ptr %wyrm_arena_")), wyrm_var_arena_name), val_string(", ptr ")), wyrm_var_size_reg), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("ArenaReset")))) {
    Value wyrm_var_t = wyrm_fn_gen_temp();
    Value wyrm_var_arena_name = val_array_get(wyrm_var_node, val_number(1));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_arena_reset(ptr "), wyrm_var_t), val_string(", ptr %wyrm_arena_")), wyrm_var_arena_name), val_string(")")));
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
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("weak")))) {
    wyrm_var_builtin_func = val_string("llvm_val_weak_ref");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("lock")))) {
    wyrm_var_builtin_func = val_string("llvm_val_weak_lock");
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
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("json_parse")))) {
    wyrm_var_builtin_func = val_string("llvm_val_json_parse");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("json_encode")))) {
    wyrm_var_builtin_func = val_string("llvm_val_json_encode");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("json_pretty")))) {
    wyrm_var_builtin_func = val_string("llvm_val_json_pretty");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("json_get")))) {
    wyrm_var_builtin_func = val_string("llvm_val_json_get");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("json_has")))) {
    wyrm_var_builtin_func = val_string("llvm_val_json_has");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("json_set")))) {
    wyrm_var_builtin_func = val_string("llvm_val_json_set");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("json_object")))) {
    wyrm_var_builtin_func = val_string("llvm_val_json_object");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("yaml_parse")))) {
    wyrm_var_builtin_func = val_string("llvm_val_yaml_parse");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("yaml_encode")))) {
    wyrm_var_builtin_func = val_string("llvm_val_yaml_encode");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("map_new")))) {
    wyrm_var_builtin_func = val_string("llvm_val_map_new");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("map_set")))) {
    wyrm_var_builtin_func = val_string("llvm_val_map_set");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("map_get")))) {
    wyrm_var_builtin_func = val_string("llvm_val_map_get");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("map_has")))) {
    wyrm_var_builtin_func = val_string("llvm_val_map_has");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("map_del")))) {
    wyrm_var_builtin_func = val_string("llvm_val_map_del");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("map_keys")))) {
    wyrm_var_builtin_func = val_string("llvm_val_map_keys");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("map_values")))) {
    wyrm_var_builtin_func = val_string("llvm_val_map_values");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("map_len")))) {
    wyrm_var_builtin_func = val_string("llvm_val_map_len");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("set_new")))) {
    wyrm_var_builtin_func = val_string("llvm_val_set_new");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("set_add")))) {
    wyrm_var_builtin_func = val_string("llvm_val_set_add");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("set_has")))) {
    wyrm_var_builtin_func = val_string("llvm_val_set_has");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("set_del")))) {
    wyrm_var_builtin_func = val_string("llvm_val_set_del");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("set_union")))) {
    wyrm_var_builtin_func = val_string("llvm_val_set_union");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("set_intersect")))) {
    wyrm_var_builtin_func = val_string("llvm_val_set_intersect");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("set_to_array")))) {
    wyrm_var_builtin_func = val_string("llvm_val_set_to_array");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_init")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_init");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_quit")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_quit");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_window")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_window");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_destroy_window")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_destroy_window");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_poll_event")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_poll_event");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_clear")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_clear");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_present")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_present");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_draw_rect")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_draw_rect");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_draw_line")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_draw_line");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_delay")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_delay");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("sdl_ticks")))) {
    wyrm_var_builtin_func = val_string("llvm_val_sdl_ticks");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("ffi_open")))) {
    wyrm_var_builtin_func = val_string("llvm_val_ffi_open");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("ffi_sym")))) {
    wyrm_var_builtin_func = val_string("llvm_val_ffi_sym");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("ffi_call")))) {
    wyrm_var_builtin_func = val_string("llvm_val_ffi_call");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("ffi_close")))) {
    wyrm_var_builtin_func = val_string("llvm_val_ffi_close");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_seed")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_seed");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_int")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_int");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_range")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_range");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_choice")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_choice");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_shuffle")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_shuffle");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_secure")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_secure");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_secure_int")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_secure_int");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_bytes_hex")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_bytes_hex");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_has_trng")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_has_trng");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_trng")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_trng");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_trng_int")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_trng_int");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("rand_reseed_trng")))) {
    wyrm_var_builtin_func = val_string("llvm_val_rand_reseed_trng");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_now")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_now");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_unix")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_unix");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_unix_ms")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_unix_ms");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_monotonic")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_monotonic");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_monotonic_ms")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_monotonic_ms");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_monotonic_ns")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_monotonic_ns");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_sleep")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_sleep");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_diff")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_diff");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_format")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_format");
    wyrm_var_is_builtin = val_bool(true);
    }
    else if (val_to_bool(val_eq(wyrm_var_name, val_string("time_format_local")))) {
    wyrm_var_builtin_func = val_string("llvm_val_time_format_local");
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
    if (val_to_bool(val_eq(wyrm_var_type, val_string("MemberAccess")))) {
    Value wyrm_var_obj_node = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_member = val_array_get(wyrm_var_node, val_number(2));
    Value wyrm_var_obj_reg = wyrm_fn_gen_expr(wyrm_var_obj_node);
    Value wyrm_var_member_const = wyrm_fn_get_llvm_str_constant(wyrm_var_member);
    Value wyrm_var_t = wyrm_fn_gen_temp();
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_struct_get(ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_obj_reg), val_string(", ptr ")), wyrm_var_member_const), val_string(")")));
    return wyrm_var_t;
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("MethodCall")))) {
    Value wyrm_var_obj_node = val_array_get(wyrm_var_node, val_number(1));
    Value wyrm_var_method = val_array_get(wyrm_var_node, val_number(2));
    Value wyrm_var_args = val_array_get(wyrm_var_node, val_number(3));
    Value wyrm_var_obj_reg = wyrm_fn_gen_expr(wyrm_var_obj_node);
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
    Value wyrm_var_call_args = val_add(val_add(val_add(val_string("ptr "), wyrm_var_t), val_string(", ptr ")), wyrm_var_obj_reg);
    Value wyrm_var_j = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_num_args))) {
    break;
    }
    wyrm_var_call_args = val_add(val_add(wyrm_var_call_args, val_string(", ptr ")), val_array_get(wyrm_var_arg_regs, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_num_args)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @wyrm_fn_"), wyrm_var_method), val_string("(")), wyrm_var_call_args), val_string(")")));
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
    if (val_to_bool(val_eq(wyrm_var_type, val_string("MemberAssign")))) {
    Value wyrm_var_obj_node = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_member = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_val = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_obj_reg = wyrm_fn_gen_expr(wyrm_var_obj_node);
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_val);
    Value wyrm_var_member_const = wyrm_fn_get_llvm_str_constant(wyrm_var_member);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_struct_set(ptr "), wyrm_var_obj_reg), val_string(", ptr ")), wyrm_var_member_const), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("StructDef")))) {
    Value wyrm_var_s_name = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_fields = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_methods = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_num_f = val_len(wyrm_var_fields);
    Value wyrm_var_params_decl = val_string("ptr %result_ptr");
    Value wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_f))) {
    break;
    }
    Value wyrm_var_f_info = val_array_get(wyrm_var_fields, wyrm_var_i);
    Value wyrm_var_f_name = wyrm_var_f_info;
    if (val_to_bool(wyrm_fn_is_array(wyrm_var_f_info))) {
    wyrm_var_f_name = val_array_get(wyrm_var_f_info, val_number(0));
    }
    wyrm_var_params_decl = val_add(val_add(wyrm_var_params_decl, val_string(", ptr %arg_")), wyrm_var_f_name);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_f)));
    Value wyrm_var_ctor_code = val_add(val_add(val_add(val_add(val_string("define void @wyrm_fn_"), wyrm_var_s_name), val_string("(")), wyrm_var_params_decl), val_string(") {\n"));
    wyrm_var_ctor_code = val_add(wyrm_var_ctor_code, val_string("  %s_ptr = alloca %struct.Value, align 8\n"));
    Value wyrm_var_s_name_const = wyrm_fn_get_llvm_str_constant(wyrm_var_s_name);
    wyrm_var_ctor_code = val_add(val_add(val_add(val_add(val_add(wyrm_var_ctor_code, val_string("  call void @llvm_val_struct_create(ptr %s_ptr, ptr ")), wyrm_var_s_name_const), val_string(", i32 ")), val_str(wyrm_var_num_f)), val_string(")\n"));
    wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_f))) {
    break;
    }
    Value wyrm_var_f_info = val_array_get(wyrm_var_fields, wyrm_var_i);
    Value wyrm_var_f_name = wyrm_var_f_info;
    if (val_to_bool(wyrm_fn_is_array(wyrm_var_f_info))) {
    wyrm_var_f_name = val_array_get(wyrm_var_f_info, val_number(0));
    }
    Value wyrm_var_f_const = wyrm_fn_get_llvm_str_constant(wyrm_var_f_name);
    wyrm_var_ctor_code = val_add(val_add(val_add(val_add(val_add(wyrm_var_ctor_code, val_string("  call void @llvm_val_struct_set(ptr %s_ptr, ptr ")), wyrm_var_f_const), val_string(", ptr %arg_")), wyrm_var_f_name), val_string(")\n"));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_f)));
    wyrm_var_ctor_code = val_add(wyrm_var_ctor_code, val_string("  call void @llvm_val_copy(ptr %result_ptr, ptr %s_ptr)\n"));
    wyrm_var_ctor_code = val_add(wyrm_var_ctor_code, val_string("  call void @llvm_val_drop(ptr %s_ptr)\n"));
    wyrm_var_ctor_code = val_add(wyrm_var_ctor_code, val_string("  ret void\n}\n"));
    val_array_append(wyrm_var_g_llvm_funcs, wyrm_var_ctor_code);
    Value wyrm_var_m_idx = val_number(0);
    Value wyrm_var_num_m = val_len(wyrm_var_methods);
    do {
    if (val_to_bool(val_ge(wyrm_var_m_idx, wyrm_var_num_m))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_methods, wyrm_var_m_idx));
    wyrm_var_m_idx = val_add(wyrm_var_m_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_m_idx, wyrm_var_num_m)));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("FuncDef")))) {
    val_array_set(wyrm_var_in_function, val_number(0), val_bool(true));
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_params = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_body = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_outer_types = wyrm_var_current_local_types;
    wyrm_fn_analyze_scope_types(wyrm_var_body, wyrm_var_params);
    val_array_set(wyrm_var_g_allocas, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_g_insts, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_temp_count, val_number(0), val_number(0));
    val_array_set(wyrm_var_g_scopes, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_declared_locals, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_g_func_owned, val_number(0), val_array_init(0));
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
    wyrm_var_current_local_types = wyrm_var_outer_types;
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("VarDecl")))) {
    Value wyrm_var_decl_kind = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_val = val_array_get(wyrm_var_stmt, val_number(3));
    Value wyrm_var_explicit_t = val_string("");
    if (val_to_bool(val_gt(val_len(wyrm_var_stmt), val_number(4)))) {
    wyrm_var_explicit_t = val_array_get(wyrm_var_stmt, val_number(4));
    }
    Value wyrm_var_v_type = wyrm_fn_canonical_type(wyrm_fn_get_var_type(wyrm_var_name));
    if (val_to_bool(val_ne(wyrm_var_explicit_t, val_string("")))) {
    wyrm_var_v_type = wyrm_fn_canonical_type(wyrm_var_explicit_t);
    wyrm_fn_set_var_type(wyrm_var_name, wyrm_var_v_type);
    }
    if (val_to_bool(val_eq(wyrm_var_decl_kind, val_string("owned")))) {
    val_array_append(val_array_get(wyrm_var_g_func_owned, val_number(0)), wyrm_var_name);
    }
    if (val_to_bool(wyrm_fn_is_unboxed_type(wyrm_var_v_type))) {
    Value wyrm_var_llvm_t = wyrm_fn_get_llvm_type(wyrm_var_v_type);
    Value wyrm_var_llvm_a = wyrm_fn_get_llvm_align(wyrm_var_v_type);
    Value wyrm_var_target_ptr = val_string("");
    if (val_to_bool(val_array_get(wyrm_var_in_function, val_number(0)))) {
    if (val_to_bool(val_not(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_locals, val_number(0)), wyrm_var_name)))) {
    val_array_append(val_array_get(wyrm_var_g_allocas, val_number(0)), val_add(val_add(val_add(val_add(val_add(val_string("  %wyrm_var_"), wyrm_var_name), val_string(" = alloca ")), wyrm_var_llvm_t), val_string(", align ")), wyrm_var_llvm_a));
    val_array_append(val_array_get(wyrm_var_declared_locals, val_number(0)), wyrm_var_name);
    }
    wyrm_var_target_ptr = val_add(val_string("%wyrm_var_"), wyrm_var_name);
    }
    else {
    if (val_to_bool(val_not(wyrm_fn_contains_val(val_array_get(wyrm_var_emitted_globals, val_number(0)), wyrm_var_name)))) {
    Value wyrm_var_init_val = val_string("0");
    if (val_to_bool(val_or(val_eq(wyrm_var_llvm_t, val_string("double")), val_eq(wyrm_var_llvm_t, val_string("float"))))) {
    wyrm_var_init_val = val_string("0.0");
    }
    else if (val_to_bool(val_eq(wyrm_var_llvm_t, val_string("ptr")))) {
    wyrm_var_init_val = val_string("null");
    }
    val_array_append(wyrm_var_g_llvm_globals, val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_string("@wyrm_var_"), wyrm_var_name), val_string(" = global ")), wyrm_var_llvm_t), val_string(" ")), wyrm_var_init_val), val_string(", align ")), wyrm_var_llvm_a));
    val_array_append(val_array_get(wyrm_var_emitted_globals, val_number(0)), wyrm_var_name);
    }
    wyrm_var_target_ptr = val_add(val_string("@wyrm_var_"), wyrm_var_name);
    }
    if (val_to_bool(wyrm_fn_can_gen_unboxed(wyrm_var_val, wyrm_var_v_type))) {
    Value wyrm_var_val_reg = wyrm_fn_gen_unboxed_expr(wyrm_var_val, wyrm_var_v_type);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_string("store "), wyrm_var_llvm_t), val_string(" ")), wyrm_var_val_reg), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align ")), wyrm_var_llvm_a));
    }
    else {
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_val);
    Value wyrm_var_union_ptr = val_add(val_string("%union_ptr_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_dbl_val = val_add(val_string("%dbl_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_union_ptr, val_string(" = getelementptr %struct.Value, ptr ")), wyrm_var_val_reg), val_string(", i32 0, i32 1")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_dbl_val, val_string(" = load double, ptr ")), wyrm_var_union_ptr), val_string(", align 8")));
    if (val_to_bool(val_eq(wyrm_var_v_type, val_string("i32")))) {
    Value wyrm_var_i_val = val_add(val_string("%i32_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_i_val, val_string(" = fptosi double ")), wyrm_var_dbl_val), val_string(" to i32")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store i32 "), wyrm_var_i_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 4")));
    }
    else if (val_to_bool(val_eq(wyrm_var_v_type, val_string("i64")))) {
    Value wyrm_var_i_val = val_add(val_string("%i64_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_i_val, val_string(" = fptosi double ")), wyrm_var_dbl_val), val_string(" to i64")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store i64 "), wyrm_var_i_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 8")));
    }
    else if (val_to_bool(val_eq(wyrm_var_v_type, val_string("u8")))) {
    Value wyrm_var_i_val = val_add(val_string("%u8_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_i_val, val_string(" = fptoui double ")), wyrm_var_dbl_val), val_string(" to i8")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store i8 "), wyrm_var_i_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 1")));
    }
    else if (val_to_bool(val_eq(wyrm_var_v_type, val_string("f32")))) {
    Value wyrm_var_f_val = val_add(val_string("%f32_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_f_val, val_string(" = fptrunc double ")), wyrm_var_dbl_val), val_string(" to float")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store float "), wyrm_var_f_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 4")));
    }
    else if (val_to_bool(val_eq(wyrm_var_v_type, val_string("bool")))) {
    Value wyrm_var_b_val = val_add(val_string("%b_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_b_val, val_string(" = call i1 @llvm_val_to_bool(ptr ")), wyrm_var_val_reg), val_string(")")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store i1 "), wyrm_var_b_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 1")));
    }
    else {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store double "), wyrm_var_dbl_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 8")));
    }
    }
    return val_null();
    }
    if (val_to_bool(val_array_get(wyrm_var_in_function, val_number(0)))) {
    if (val_to_bool(val_not(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_locals, val_number(0)), wyrm_var_name)))) {
    val_array_append(val_array_get(wyrm_var_g_allocas, val_number(0)), val_add(val_add(val_string("  %wyrm_var_"), wyrm_var_name), val_string(" = alloca %struct.Value, align 8")));
    val_array_append(val_array_get(wyrm_var_declared_locals, val_number(0)), wyrm_var_name);
    }
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_val);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr %wyrm_var_"), wyrm_var_name), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
    if (val_to_bool(val_and(val_and(val_ne(wyrm_var_val, val_null()), wyrm_fn_is_array(wyrm_var_val)), val_or(val_eq(val_array_get(wyrm_var_val, val_number(0)), val_string("FuncCall")), val_eq(val_array_get(wyrm_var_val, val_number(0)), val_string("WeakRef")))))) {
    wyrm_fn_emit_inst(val_add(val_add(val_string("call void @llvm_val_drop(ptr "), wyrm_var_val_reg), val_string(")")));
    }
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
    if (val_to_bool(val_not(wyrm_fn_contains_val(val_array_get(wyrm_var_emitted_globals, val_number(0)), wyrm_var_name)))) {
    val_array_append(wyrm_var_g_llvm_globals, val_add(val_add(val_string("@wyrm_var_"), wyrm_var_name), val_string(" = global %struct.Value zeroinitializer, align 8")));
    val_array_append(val_array_get(wyrm_var_emitted_globals, val_number(0)), wyrm_var_name);
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr @wyrm_var_"), wyrm_var_name), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
    }
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Assign")))) {
    Value wyrm_var_name = val_array_get(wyrm_var_stmt, val_number(1));
    Value wyrm_var_val = val_array_get(wyrm_var_stmt, val_number(2));
    Value wyrm_var_v_type = wyrm_fn_canonical_type(wyrm_fn_get_var_type(wyrm_var_name));
    Value wyrm_var_target_ptr = val_add(val_string("%wyrm_var_"), wyrm_var_name);
    if (val_to_bool(val_or(wyrm_fn_contains_val(val_array_get(wyrm_var_declared_globals, val_number(0)), wyrm_var_name), val_not(val_array_get(wyrm_var_in_function, val_number(0)))))) {
    wyrm_var_target_ptr = val_add(val_string("@wyrm_var_"), wyrm_var_name);
    }
    if (val_to_bool(wyrm_fn_is_unboxed_type(wyrm_var_v_type))) {
    Value wyrm_var_llvm_t = wyrm_fn_get_llvm_type(wyrm_var_v_type);
    Value wyrm_var_llvm_a = wyrm_fn_get_llvm_align(wyrm_var_v_type);
    if (val_to_bool(wyrm_fn_can_gen_unboxed(wyrm_var_val, wyrm_var_v_type))) {
    Value wyrm_var_val_reg = wyrm_fn_gen_unboxed_expr(wyrm_var_val, wyrm_var_v_type);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_string("store "), wyrm_var_llvm_t), val_string(" ")), wyrm_var_val_reg), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align ")), wyrm_var_llvm_a));
    }
    else {
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_val);
    Value wyrm_var_union_ptr = val_add(val_string("%union_ptr_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    Value wyrm_var_dbl_val = val_add(val_string("%dbl_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_union_ptr, val_string(" = getelementptr %struct.Value, ptr ")), wyrm_var_val_reg), val_string(", i32 0, i32 1")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_dbl_val, val_string(" = load double, ptr ")), wyrm_var_union_ptr), val_string(", align 8")));
    if (val_to_bool(val_eq(wyrm_var_v_type, val_string("i32")))) {
    Value wyrm_var_i_val = val_add(val_string("%i32_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_i_val, val_string(" = fptosi double ")), wyrm_var_dbl_val), val_string(" to i32")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store i32 "), wyrm_var_i_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 4")));
    }
    else if (val_to_bool(val_eq(wyrm_var_v_type, val_string("i64")))) {
    Value wyrm_var_i_val = val_add(val_string("%i64_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_i_val, val_string(" = fptosi double ")), wyrm_var_dbl_val), val_string(" to i64")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store i64 "), wyrm_var_i_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 8")));
    }
    else if (val_to_bool(val_eq(wyrm_var_v_type, val_string("u8")))) {
    Value wyrm_var_i_val = val_add(val_string("%u8_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_i_val, val_string(" = fptoui double ")), wyrm_var_dbl_val), val_string(" to i8")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store i8 "), wyrm_var_i_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 1")));
    }
    else if (val_to_bool(val_eq(wyrm_var_v_type, val_string("f32")))) {
    Value wyrm_var_f_val = val_add(val_string("%f32_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_f_val, val_string(" = fptrunc double ")), wyrm_var_dbl_val), val_string(" to float")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store float "), wyrm_var_f_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 4")));
    }
    else if (val_to_bool(val_eq(wyrm_var_v_type, val_string("bool")))) {
    Value wyrm_var_b_val = val_add(val_string("%b_val_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_b_val, val_string(" = call i1 @llvm_val_to_bool(ptr ")), wyrm_var_val_reg), val_string(")")));
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store i1 "), wyrm_var_b_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 1")));
    }
    else {
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("store double "), wyrm_var_dbl_val), val_string(", ptr ")), wyrm_var_target_ptr), val_string(", align 8")));
    }
    }
    return val_null();
    }
    Value wyrm_var_val_reg = wyrm_fn_gen_expr(wyrm_var_val);
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_string("call void @llvm_val_copy(ptr "), wyrm_var_target_ptr), val_string(", ptr ")), wyrm_var_val_reg), val_string(")")));
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
    Value wyrm_var_num_elifs = val_len(wyrm_var_elif_clauses);
    Value wyrm_var_cond_i1 = val_add(val_string("%cond_i1_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    if (val_to_bool(wyrm_fn_can_gen_static_cmp(wyrm_var_cond))) {
    wyrm_var_cond_i1 = wyrm_fn_gen_static_cmp(wyrm_var_cond);
    }
    else {
    Value wyrm_var_cond_reg = wyrm_fn_gen_expr(wyrm_var_cond);
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_cond_i1, val_string(" = call i1 @llvm_val_to_bool(ptr ")), wyrm_var_cond_reg), val_string(")")));
    }
    Value wyrm_var_label_idx = val_array_get(wyrm_var_temp_count, val_number(0));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    Value wyrm_var_then_label = val_add(val_string("then_"), val_str(wyrm_var_label_idx));
    Value wyrm_var_merge_label = val_add(val_string("merge_"), val_str(wyrm_var_label_idx));
    Value wyrm_var_first_false_label = wyrm_var_merge_label;
    if (val_to_bool(val_gt(wyrm_var_num_elifs, val_number(0)))) {
    wyrm_var_first_false_label = val_add(val_add(val_string("elif_cond_"), val_str(wyrm_var_label_idx)), val_string("_0"));
    }
    else if (val_to_bool(val_gt(val_len(wyrm_var_else_body), val_number(0)))) {
    wyrm_var_first_false_label = val_add(val_string("else_"), val_str(wyrm_var_label_idx));
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_string("br i1 "), wyrm_var_cond_i1), val_string(", label %")), wyrm_var_then_label), val_string(", label %")), wyrm_var_first_false_label));
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
    wyrm_var_i = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_elifs))) {
    break;
    }
    Value wyrm_var_clause = val_array_get(wyrm_var_elif_clauses, wyrm_var_i);
    Value wyrm_var_elif_cond = val_array_get(wyrm_var_clause, val_number(0));
    Value wyrm_var_elif_body = val_array_get(wyrm_var_clause, val_number(1));
    Value wyrm_var_current_cond_label = val_add(val_add(val_add(val_string("elif_cond_"), val_str(wyrm_var_label_idx)), val_string("_")), val_str(wyrm_var_i));
    Value wyrm_var_current_then_label = val_add(val_add(val_add(val_string("elif_then_"), val_str(wyrm_var_label_idx)), val_string("_")), val_str(wyrm_var_i));
    Value wyrm_var_next_false_label = wyrm_var_merge_label;
    if (val_to_bool(val_lt(val_add(wyrm_var_i, val_number(1)), wyrm_var_num_elifs))) {
    wyrm_var_next_false_label = val_add(val_add(val_add(val_string("elif_cond_"), val_str(wyrm_var_label_idx)), val_string("_")), val_str(val_add(wyrm_var_i, val_number(1))));
    }
    else if (val_to_bool(val_gt(val_len(wyrm_var_else_body), val_number(0)))) {
    wyrm_var_next_false_label = val_add(val_string("else_"), val_str(wyrm_var_label_idx));
    }
    wyrm_fn_emit_label(val_add(wyrm_var_current_cond_label, val_string(":")));
    Value wyrm_var_current_cond_i1 = val_add(val_string("%cond_i1_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    if (val_to_bool(wyrm_fn_can_gen_static_cmp(wyrm_var_elif_cond))) {
    wyrm_var_current_cond_i1 = wyrm_fn_gen_static_cmp(wyrm_var_elif_cond);
    }
    else {
    Value wyrm_var_current_cond_reg = wyrm_fn_gen_expr(wyrm_var_elif_cond);
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_current_cond_i1, val_string(" = call i1 @llvm_val_to_bool(ptr ")), wyrm_var_current_cond_reg), val_string(")")));
    }
    wyrm_fn_emit_inst(val_add(val_add(val_add(val_add(val_add(val_string("br i1 "), wyrm_var_current_cond_i1), val_string(", label %")), wyrm_var_current_then_label), val_string(", label %")), wyrm_var_next_false_label));
    wyrm_fn_emit_label(val_add(wyrm_var_current_then_label, val_string(":")));
    wyrm_fn_push_scope();
    Value wyrm_var_j = val_number(0);
    Value wyrm_var_elif_len = val_len(wyrm_var_elif_body);
    do {
    if (val_to_bool(val_ge(wyrm_var_j, wyrm_var_elif_len))) {
    break;
    }
    wyrm_fn_gen_statement(val_array_get(wyrm_var_elif_body, wyrm_var_j));
    wyrm_var_j = val_add(wyrm_var_j, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_j, wyrm_var_elif_len)));
    wyrm_fn_pop_scope();
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_merge_label));
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_num_elifs)));
    if (val_to_bool(val_gt(val_len(wyrm_var_else_body), val_number(0)))) {
    Value wyrm_var_else_label = val_add(val_string("else_"), val_str(wyrm_var_label_idx));
    wyrm_fn_emit_label(val_add(wyrm_var_else_label, val_string(":")));
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
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_merge_label));
    }
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
    val_array_append(val_array_get(wyrm_var_g_loop_breaks, val_number(0)), wyrm_var_merge_label);
    val_array_append(val_array_get(wyrm_var_g_loop_continues, val_number(0)), wyrm_var_cond_label);
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
    val_array_pop(val_array_get(wyrm_var_g_loop_breaks, val_number(0)));
    val_array_pop(val_array_get(wyrm_var_g_loop_continues, val_number(0)));
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_cond_label));
    wyrm_fn_emit_label(val_add(wyrm_var_cond_label, val_string(":")));
    Value wyrm_var_cond_i1 = val_add(val_string("%cond_i1_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    if (val_to_bool(wyrm_fn_can_gen_static_cmp(wyrm_var_cond))) {
    wyrm_var_cond_i1 = wyrm_fn_gen_static_cmp(wyrm_var_cond);
    }
    else {
    Value wyrm_var_cond_reg = wyrm_fn_gen_expr(wyrm_var_cond);
    wyrm_fn_emit_inst(val_add(val_add(val_add(wyrm_var_cond_i1, val_string(" = call i1 @llvm_val_to_bool(ptr ")), wyrm_var_cond_reg), val_string(")")));
    }
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
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Break")))) {
    wyrm_fn_gen_all_cleanups();
    Value wyrm_var_size = val_len(val_array_get(wyrm_var_g_loop_breaks, val_number(0)));
    if (val_to_bool(val_gt(wyrm_var_size, val_number(0)))) {
    Value wyrm_var_breaks_list = val_array_get(wyrm_var_g_loop_breaks, val_number(0));
    Value wyrm_var_target = val_array_get(wyrm_var_breaks_list, val_sub(wyrm_var_size, val_number(1)));
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_target));
    }
    Value wyrm_var_dead_block = val_add(val_string("dead_block_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_label(val_add(wyrm_var_dead_block, val_string(":")));
    return val_null();
    }
    if (val_to_bool(val_eq(wyrm_var_type, val_string("Continue")))) {
    wyrm_fn_gen_all_cleanups();
    Value wyrm_var_size = val_len(val_array_get(wyrm_var_g_loop_continues, val_number(0)));
    if (val_to_bool(val_gt(wyrm_var_size, val_number(0)))) {
    Value wyrm_var_continues_list = val_array_get(wyrm_var_g_loop_continues, val_number(0));
    Value wyrm_var_target = val_array_get(wyrm_var_continues_list, val_sub(wyrm_var_size, val_number(1)));
    wyrm_fn_emit_inst(val_add(val_string("br label %"), wyrm_var_target));
    }
    Value wyrm_var_dead_block = val_add(val_string("dead_block_"), val_str(val_array_get(wyrm_var_temp_count, val_number(0))));
    val_array_set(wyrm_var_temp_count, val_number(0), val_add(val_array_get(wyrm_var_temp_count, val_number(0)), val_number(1)));
    wyrm_fn_emit_label(val_add(wyrm_var_dead_block, val_string(":")));
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
    if (val_to_bool(val_not(wyrm_fn_is_array(wyrm_var_expr)))) {
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
    if (val_to_bool(val_and(val_and(val_and(val_and(wyrm_var_is_err_func, val_not(wyrm_var_in_try_stmt)), val_not(wyrm_var_in_try_expr)), val_not(wyrm_var_in_catch_expr)), val_not(wyrm_var_func_returns_error)))) {
    wyrm_fn_report_diagnostic(val_string("E0004"), val_add(val_add(val_string("unhandled error: function '"), wyrm_var_name), val_string("' returns !Error but is called without try/catch handling")), val_number(0), val_number(0), val_string("wrap in try or annotate calling function with !Error"));
    }
    Value wyrm_var_args = val_array_get(wyrm_var_expr, val_number(2));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_al = val_len(wyrm_var_args);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_al))) {
    break;
    }
    wyrm_fn_check_expr(val_array_get(wyrm_var_args, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_in_try_expr, wyrm_var_in_catch_expr, wyrm_var_func_returns_error);
    wyrm_var_i = val_add(wyrm_var_i, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_i, wyrm_var_al)));
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("TryExpr")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, val_bool(true), wyrm_var_in_catch_expr, wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("CatchExpr")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_in_try_expr, val_bool(true), wyrm_var_func_returns_error);
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_in_try_expr, val_bool(false), wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("BinaryOp")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_in_try_expr, wyrm_var_in_catch_expr, wyrm_var_func_returns_error);
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(3)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_in_try_expr, wyrm_var_in_catch_expr, wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("UnaryOp")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(2)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_in_try_expr, wyrm_var_in_catch_expr, wyrm_var_func_returns_error);
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("Array")))) {
    Value wyrm_var_elems = val_array_get(wyrm_var_expr, val_number(1));
    Value wyrm_var_i = val_number(0);
    Value wyrm_var_el = val_len(wyrm_var_elems);
    do {
    if (val_to_bool(val_ge(wyrm_var_i, wyrm_var_el))) {
    break;
    }
    wyrm_fn_check_expr(val_array_get(wyrm_var_elems, wyrm_var_i), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_in_try_expr, wyrm_var_in_catch_expr, wyrm_var_func_returns_error);
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
    }
    else if (val_to_bool(val_eq(wyrm_var_n_type, val_string("WeakRef")))) {
    wyrm_fn_check_expr(val_array_get(wyrm_var_expr, val_number(1)), wyrm_var_error_funcs, wyrm_var_in_try_stmt, wyrm_var_in_try_expr, wyrm_var_in_catch_expr, wyrm_var_func_returns_error);
    }
    return val_null();
}

Value wyrm_fn_check_stmt(Value wyrm_var_stmt, Value wyrm_var_error_funcs, Value wyrm_var_in_try_stmt, Value wyrm_var_func_returns_error) {
    if (val_to_bool(val_not(wyrm_fn_is_array(wyrm_var_stmt)))) {
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
    Value wyrm_var_error_funcs = val_array_init(0);
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
    wyrm_var_g_llvm_globals = val_array_init(0);
    wyrm_var_g_llvm_decls = val_array_init(0);
    wyrm_var_g_llvm_funcs = val_array_init(0);
    wyrm_var_g_llvm_main = val_array_init(0);
    val_array_set(wyrm_var_has_main_def, val_number(0), val_bool(false));
    val_array_set(wyrm_var_temp_count, val_number(0), val_number(0));
    val_array_append(wyrm_var_g_llvm_globals, val_string("; Module generated by Wyrm 3.2.0 Self-Hosted LLVM backend"));
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
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_arena_alloc(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_arena_reset(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_read_file(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_error_val(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_write_file(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_exit(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_system(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_getenv(ptr, ptr)"));
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
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_struct_create(ptr, ptr, i32)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_struct_get(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_struct_set(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_weak_ref(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_weak_lock(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_from_i64(ptr, i64)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_from_u8(ptr, i8)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_from_f32(ptr, float)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_from_bool(ptr, i1)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_drop(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_json_parse(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_json_encode(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_json_pretty(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_json_get(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_json_has(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_json_set(ptr, ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_json_object(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_yaml_parse(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_yaml_encode(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_map_new(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_map_set(ptr, ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_map_get(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_map_has(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_map_del(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_map_keys(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_map_values(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_map_len(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_set_new(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_set_add(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_set_has(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_set_del(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_set_union(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_set_intersect(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_set_to_array(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_init(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_quit(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_window(ptr, ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_destroy_window(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_poll_event(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_clear(ptr, ptr, ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_present(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_draw_rect(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_draw_line(ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_delay(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_sdl_ticks(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_ffi_open(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_ffi_sym(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_ffi_call(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_ffi_close(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_seed(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_int(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_range(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_choice(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_shuffle(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_secure(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_secure_int(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_bytes_hex(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_has_trng(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_trng(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_trng_int(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_rand_reseed_trng(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_now(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_unix(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_unix_ms(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_monotonic(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_monotonic_ms(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_monotonic_ns(ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_sleep(ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_diff(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_format(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("declare void @llvm_val_time_format_local(ptr, ptr, ptr)"));
    val_array_append(wyrm_var_g_llvm_decls, val_string("@wyrm_sys_args = external global %struct.Value, align 8"));
    val_array_set(wyrm_var_declared_globals, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_emitted_globals, val_number(0), val_array_init(0));
    wyrm_var_global_var_types = val_array_init(0);
    val_array_set(wyrm_var_g_main_allocas, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_g_loop_breaks, val_number(0), val_array_init(0));
    val_array_set(wyrm_var_g_loop_continues, val_number(0), val_array_init(0));
    wyrm_fn_collect_globals_from_stmts(wyrm_var_ast);
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
    wyrm_fn_analyze_scope_types(wyrm_var_ast, val_array_init(0));
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

Value wyrm_fn_parse_file(Value wyrm_var_path) {
    if (val_to_bool(wyrm_fn_contains_val(val_array_get(wyrm_var_g_processed_files, val_number(0)), wyrm_var_path))) {
    return val_array_init(0);
    }
    val_array_append(val_array_get(wyrm_var_g_processed_files, val_number(0)), wyrm_var_path);
    if (val_to_bool(val_or(val_or(val_or(val_or(val_or(val_or(val_or(val_eq(wyrm_var_path, val_string("std.json")), val_eq(wyrm_var_path, val_string("std.yaml"))), val_eq(wyrm_var_path, val_string("std.collections"))), val_eq(wyrm_var_path, val_string("std.sdl"))), val_eq(wyrm_var_path, val_string("std.ffi"))), val_eq(wyrm_var_path, val_string("std.thread"))), val_eq(wyrm_var_path, val_string("std.random"))), val_eq(wyrm_var_path, val_string("std.time"))))) {
    return val_array_init(0);
    }
    Value wyrm_var_alt_path = val_string("");
    Value wyrm_var_pi = val_number(0);
    Value wyrm_var_plen = val_len(wyrm_var_path);
    do {
    if (val_to_bool(val_ge(wyrm_var_pi, wyrm_var_plen))) {
    break;
    }
    Value wyrm_var_ch = val_char_at(wyrm_var_path, wyrm_var_pi);
    if (val_to_bool(val_eq(wyrm_var_ch, val_string(".")))) {
    wyrm_var_alt_path = val_add(wyrm_var_alt_path, val_string("/"));
    }
    else {
    wyrm_var_alt_path = val_add(wyrm_var_alt_path, wyrm_var_ch);
    }
    wyrm_var_pi = val_add(wyrm_var_pi, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_pi, wyrm_var_plen)));
    Value wyrm_var_current_dir = val_string("");
    Value wyrm_var_cur_f = val_array_get(wyrm_var_current_source_file, val_number(0));
    Value wyrm_var_last_slash = val_sub(val_number(0.0), val_number(1));
    Value wyrm_var_ci = val_number(0);
    Value wyrm_var_clen = val_len(wyrm_var_cur_f);
    do {
    if (val_to_bool(val_ge(wyrm_var_ci, wyrm_var_clen))) {
    break;
    }
    Value wyrm_var_cc = val_char_at(wyrm_var_cur_f, wyrm_var_ci);
    if (val_to_bool(val_or(val_eq(wyrm_var_cc, val_string("/")), val_eq(wyrm_var_cc, val_string("\\"))))) {
    wyrm_var_last_slash = wyrm_var_ci;
    }
    wyrm_var_ci = val_add(wyrm_var_ci, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_ci, wyrm_var_clen)));
    if (val_to_bool(val_ne(wyrm_var_last_slash, val_sub(val_number(0.0), val_number(1))))) {
    Value wyrm_var_si = val_number(0);
    do {
    if (val_to_bool(val_ge(wyrm_var_si, wyrm_var_last_slash))) {
    break;
    }
    wyrm_var_current_dir = val_add(wyrm_var_current_dir, val_char_at(wyrm_var_cur_f, wyrm_var_si));
    wyrm_var_si = val_add(wyrm_var_si, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_si, wyrm_var_last_slash)));
    }
    Value wyrm_var_resolved_file = val_string("");
    Value wyrm_var_content = val_null();
    Value wyrm_var_candidates = val_array_init(0);
    val_array_append(wyrm_var_candidates, wyrm_var_path);
    val_array_append(wyrm_var_candidates, val_add(wyrm_var_path, val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(wyrm_var_path, val_string("/mod.wyr")));
    val_array_append(wyrm_var_candidates, val_add(wyrm_var_path, val_string("/main.wyr")));
    val_array_append(wyrm_var_candidates, wyrm_var_alt_path);
    val_array_append(wyrm_var_candidates, val_add(wyrm_var_alt_path, val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(wyrm_var_alt_path, val_string("/mod.wyr")));
    val_array_append(wyrm_var_candidates, val_add(wyrm_var_alt_path, val_string("/main.wyr")));
    if (val_to_bool(val_ne(wyrm_var_current_dir, val_string("")))) {
    val_array_append(wyrm_var_candidates, val_add(val_add(wyrm_var_current_dir, val_string("/")), wyrm_var_path));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_add(wyrm_var_current_dir, val_string("/")), wyrm_var_path), val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_add(wyrm_var_current_dir, val_string("/")), wyrm_var_path), val_string("/mod.wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(wyrm_var_current_dir, val_string("/")), wyrm_var_alt_path));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_add(wyrm_var_current_dir, val_string("/")), wyrm_var_alt_path), val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_add(wyrm_var_current_dir, val_string("/")), wyrm_var_alt_path), val_string("/mod.wyr")));
    }
    val_array_append(wyrm_var_candidates, val_add(val_string("library/"), wyrm_var_path));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("library/"), wyrm_var_path), val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("library/"), wyrm_var_path), val_string("/mod.wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("library/"), wyrm_var_path), val_string("/main.wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_string("library/"), wyrm_var_alt_path));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("library/"), wyrm_var_alt_path), val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("library/"), wyrm_var_alt_path), val_string("/mod.wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_string("packages/"), wyrm_var_path));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("packages/"), wyrm_var_path), val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("packages/"), wyrm_var_path), val_string("/mod.wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("packages/"), wyrm_var_path), val_string("/main.wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_string("packages/"), wyrm_var_alt_path));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("packages/"), wyrm_var_alt_path), val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_string("packages/"), wyrm_var_alt_path), val_string("/mod.wyr")));
    Value wyrm_var_home = val_getenv(val_string("USERPROFILE"));
    if (val_to_bool(val_eq(wyrm_var_home, val_null()))) {
    wyrm_var_home = val_getenv(val_string("HOME"));
    }
    if (val_to_bool(val_ne(wyrm_var_home, val_null()))) {
    val_array_append(wyrm_var_candidates, val_add(val_add(wyrm_var_home, val_string("/.wyrm/packages/")), wyrm_var_path));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_add(wyrm_var_home, val_string("/.wyrm/packages/")), wyrm_var_path), val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_add(wyrm_var_home, val_string("/.wyrm/packages/")), wyrm_var_path), val_string("/mod.wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(wyrm_var_home, val_string("/.wyrm/library/")), wyrm_var_path));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_add(wyrm_var_home, val_string("/.wyrm/library/")), wyrm_var_path), val_string(".wyr")));
    val_array_append(wyrm_var_candidates, val_add(val_add(val_add(wyrm_var_home, val_string("/.wyrm/library/")), wyrm_var_path), val_string("/mod.wyr")));
    }
    Value wyrm_var_c_idx = val_number(0);
    Value wyrm_var_c_total = val_len(wyrm_var_candidates);
    do {
    if (val_to_bool(val_ge(wyrm_var_c_idx, wyrm_var_c_total))) {
    break;
    }
    Value wyrm_var_cand = val_array_get(wyrm_var_candidates, wyrm_var_c_idx);
    Value wyrm_var_test_c = val_read_file(wyrm_var_cand);
    if (val_to_bool(val_ne(wyrm_var_test_c, val_null()))) {
    wyrm_var_content = wyrm_var_test_c;
    wyrm_var_resolved_file = wyrm_var_cand;
    break;
    }
    wyrm_var_c_idx = val_add(wyrm_var_c_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_c_idx, wyrm_var_c_total)));
    if (val_to_bool(val_eq(wyrm_var_content, val_null()))) {
    val_print(1, val_add(val_add(val_string("Compiler Error: Cannot find module '"), wyrm_var_path), val_string("'")));
    val_exit(val_number(1));
    }
    if (val_to_bool(val_and(val_ne(wyrm_var_resolved_file, val_string("")), val_ne(wyrm_var_resolved_file, wyrm_var_path)))) {
    if (val_to_bool(wyrm_fn_contains_val(val_array_get(wyrm_var_g_processed_files, val_number(0)), wyrm_var_resolved_file))) {
    return val_array_init(0);
    }
    val_array_append(val_array_get(wyrm_var_g_processed_files, val_number(0)), wyrm_var_resolved_file);
    }
    Value wyrm_var_tokens = wyrm_fn_tokenize(wyrm_var_content);
    val_array_set(wyrm_var_current_source_file, val_number(0), wyrm_var_resolved_file);
    val_array_set(wyrm_var_current_source_lines, val_number(0), val_split(wyrm_var_content, val_string("\n")));
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
    Value wyrm_var_sub_ast = wyrm_fn_parse_file(val_array_get(wyrm_var_stmt, val_number(1)));
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
    if (val_to_bool(val_lt(val_len(wyrm_var_args), val_number(2)))) {
    val_print(1, val_string("wyrmc 1.0.0"));
    val_print(1, val_string("Usage:"));
    val_print(1, val_string("  wyrmc build <file.wyr> [-o <out.exe>]"));
    val_print(1, val_string("  wyrmc run <file.wyr> [args...]"));
    val_print(1, val_string("  wyrmc --version"));
    val_print(1, val_string("  wyrmc --help"));
    val_exit(val_number(0));
    }
    Value wyrm_var_command = val_array_get(wyrm_var_args, val_number(1));
    if (val_to_bool(val_or(val_or(val_eq(wyrm_var_command, val_string("--version")), val_eq(wyrm_var_command, val_string("-v"))), val_eq(wyrm_var_command, val_string("version"))))) {
    val_print(1, val_string("wyrmc 1.0.0"));
    val_exit(val_number(0));
    }
    if (val_to_bool(val_or(val_or(val_eq(wyrm_var_command, val_string("--help")), val_eq(wyrm_var_command, val_string("-h"))), val_eq(wyrm_var_command, val_string("help"))))) {
    val_print(1, val_string("wyrmc 1.0.0"));
    val_print(1, val_string("Usage:"));
    val_print(1, val_string("  wyrmc build <file.wyr> [-o <out.exe>]"));
    val_print(1, val_string("  wyrmc run <file.wyr> [args...]"));
    val_print(1, val_string("  wyrmc --version"));
    val_print(1, val_string("  wyrmc --help"));
    val_exit(val_number(0));
    }
    if (val_to_bool(val_and(val_ne(wyrm_var_command, val_string("build")), val_ne(wyrm_var_command, val_string("run"))))) {
    val_print(1, val_add(val_add(val_string("Error: Unknown compiler command '"), wyrm_var_command), val_string("'")));
    val_print(1, val_string("Run 'wyrmc --help' for usage instructions."));
    val_exit(val_number(1));
    }
    if (val_to_bool(val_lt(val_len(wyrm_var_args), val_number(3)))) {
    val_print(1, val_string("Error: Missing input source file."));
    val_print(1, val_string("Run 'wyrmc --help' for usage instructions."));
    val_exit(val_number(1));
    }
    Value wyrm_var_source_file = val_array_get(wyrm_var_args, val_number(2));
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
    if (val_to_bool(val_eq(wyrm_var_command, val_string("run")))) {
    wyrm_var_out_exe = val_add(wyrm_var_base_name, val_string("_run.exe"));
    }
    if (val_to_bool(val_ge(val_len(wyrm_var_args), val_number(5)))) {
    if (val_to_bool(val_eq(val_array_get(wyrm_var_args, val_number(3)), val_string("-o")))) {
    wyrm_var_out_exe = val_array_get(wyrm_var_args, val_number(4));
    }
    }
    Value wyrm_var_temp_ll_file = val_add(wyrm_var_base_name, val_string("_temp.ll"));
    val_array_set(wyrm_var_g_processed_files, val_number(0), val_array_init(0));
    Value wyrm_var_ast = wyrm_fn_parse_file(wyrm_var_source_file);
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
    Value wyrm_var_ffi_c = val_add(wyrm_var_lib_path, val_string("wyrm_ffi.c"));
    Value wyrm_var_std_json_c = val_add(wyrm_var_lib_path, val_string("stdlib/wyrm_std_json.c"));
    Value wyrm_var_std_yaml_c = val_add(wyrm_var_lib_path, val_string("stdlib/wyrm_std_yaml.c"));
    Value wyrm_var_std_sdl_c = val_add(wyrm_var_lib_path, val_string("stdlib/wyrm_std_sdl.c"));
    Value wyrm_var_std_coll_c = val_add(wyrm_var_lib_path, val_string("stdlib/wyrm_std_collections.c"));
    Value wyrm_var_std_rand_c = val_add(wyrm_var_lib_path, val_string("stdlib/wyrm_std_random.c"));
    Value wyrm_var_std_time_c = val_add(wyrm_var_lib_path, val_string("stdlib/wyrm_std_time.c"));
    Value wyrm_var_all_runtime = val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_string("\""), wyrm_var_runtime_c), val_string("\" \"")), wyrm_var_arena_c), val_string("\" \"")), wyrm_var_str_c), val_string("\" \"")), wyrm_var_ffi_c), val_string("\" \"")), wyrm_var_std_json_c), val_string("\" \"")), wyrm_var_std_yaml_c), val_string("\" \"")), wyrm_var_std_sdl_c), val_string("\" \"")), wyrm_var_std_coll_c), val_string("\" \"")), wyrm_var_std_rand_c), val_string("\" \"")), wyrm_var_std_time_c), val_string("\""));
    Value wyrm_var_inc_flags = val_add(val_add(val_string("-I\""), wyrm_var_lib_path), val_string("\" -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -Wno-deprecated-declarations -Wno-switch -Wno-override-module"));
    Value wyrm_var_clang_cmd = val_string("clang");
    Value wyrm_var_test_res = val_system(val_string("clang --version > nul 2>&1"));
    if (val_to_bool(val_ne(wyrm_var_test_res, val_number(0)))) {
    Value wyrm_var_clang_check = val_read_file(val_string("C:\\Program Files\\LLVM\\bin\\clang.exe"));
    if (val_to_bool(val_ne(wyrm_var_clang_check, val_null()))) {
    wyrm_var_clang_cmd = val_string("\"C:\\Program Files\\LLVM\\bin\\clang.exe\"");
    }
    }
    Value wyrm_var_compile_cmd = val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(val_add(wyrm_var_clang_cmd, val_string(" -O2 ")), wyrm_var_inc_flags), val_string(" \"")), wyrm_var_temp_ll_file), val_string("\" ")), wyrm_var_all_runtime), val_string(" -o \"")), wyrm_var_out_exe), val_string("\""));
    Value wyrm_var_res = val_system(val_add(val_add(val_string("\""), wyrm_var_compile_cmd), val_string("\"")));
    if (val_to_bool(val_ne(wyrm_var_res, val_number(0)))) {
    val_print(1, val_string("Compilation Error: LLVM/clang failed to compile target executable."));
    val_exit(val_number(1));
    }
    if (val_to_bool(val_eq(wyrm_var_command, val_string("run")))) {
    Value wyrm_var_native_exe = val_string("");
    Value wyrm_var_p_idx = val_number(0);
    Value wyrm_var_p_len = val_len(wyrm_var_out_exe);
    do {
    if (val_to_bool(val_ge(wyrm_var_p_idx, wyrm_var_p_len))) {
    break;
    }
    Value wyrm_var_ch = val_char_at(wyrm_var_out_exe, wyrm_var_p_idx);
    if (val_to_bool(val_eq(wyrm_var_ch, val_string("/")))) {
    wyrm_var_native_exe = val_add(wyrm_var_native_exe, val_string("\\"));
    }
    else {
    wyrm_var_native_exe = val_add(wyrm_var_native_exe, wyrm_var_ch);
    }
    wyrm_var_p_idx = val_add(wyrm_var_p_idx, val_number(1));
    } while (!val_to_bool(val_ge(wyrm_var_p_idx, wyrm_var_p_len)));
    Value wyrm_var_run_cmd = val_add(val_add(val_string("\""), wyrm_var_native_exe), val_string("\""));
    Value wyrm_var_run_res = val_system(wyrm_var_run_cmd);
    val_exit(wyrm_var_run_res);
    }
    val_print(1, val_add(val_add(val_add(val_add(val_string("[wyrmc 1.0.0] Successfully compiled '"), wyrm_var_source_file), val_string("' -> Native Binary '")), wyrm_var_out_exe), val_string("'")));
    return val_null();
}

int main(int argc, char *argv[]) {
    val_init_sys_args(argc, argv);
    wyrm_var_token_pos = val_array_init(1, val_number(0));
    wyrm_var_g_tokens = val_array_init(1, val_array_init(0));
    wyrm_var_current_source_file = val_array_init(1, val_string(""));
    wyrm_var_current_source_lines = val_array_init(1, val_array_init(0));
    wyrm_var_g_processed_files = val_array_init(1, val_array_init(0));
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
    wyrm_var_emitted_globals = val_array_init(1, val_array_init(0));
    wyrm_var_declared_locals = val_array_init(1, val_array_init(0));
    wyrm_var_g_func_owned = val_array_init(1, val_array_init(0));
    wyrm_var_constants = val_array_init(0);
    wyrm_var_g_loop_breaks = val_array_init(1, val_array_init(0));
    wyrm_var_g_loop_continues = val_array_init(1, val_array_init(0));
    wyrm_var_current_local_types = val_array_init(0);
    wyrm_var_global_var_types = val_array_init(0);
    wyrm_fn_main();
    return 0;
}
