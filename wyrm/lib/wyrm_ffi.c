/*
 * wyrm_ffi.c - Wyrm Foreign Function Interface Implementation
 *
 * Implements dynamic shared library loading via dlopen (POSIX) or
 * LoadLibrary (Windows) and the four runtime FFI builtins.
 */
#include "wyrm_ffi.h"

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
