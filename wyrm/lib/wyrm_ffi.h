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
#ifndef WYRM_FFI_H
#define WYRM_FFI_H

#include "wyrm_core.h"

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

#endif /* WYRM_FFI_H */
