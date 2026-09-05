# Wyrm Language Specification

This document describes the Wyrm language design and architecture implemented in the repository. The canonical project version is stored in the [VERSION](file:///c:/Users/BEST/Desktop/wyrm/VERSION) file (currently **v3.1.0**).

---

## 1. Toolchain & Runtime

- **`wyrmc`**: The primary self-hosted compiler and execution runner implemented in pure Wyrm ([compiler/wyrmc.wyr](file:///c:/Users/BEST/Desktop/wyrm/compiler/wyrmc.wyr)). Supports ahead-of-time native binary compilation (`wyrmc build file.wyr`) and instant runner mode (`wyrmc run file.wyr`).
- **`wyrpkg`**: The project and package manager (Cargo-inspired workflow: `new`, `init`, `build`, `run`, `install`, `update`, `publish`, `remove`, `list`).
- **AOT Codegen Path**: LLVM IR compilation directly to native machine binaries via Clang / LLVM toolchain.
- **Stage 0 Bootstrap Toolchain**: Native C11 / C++20 toolchain built via [bootstrap.c](file:///c:/Users/BEST/Desktop/wyrm/wyrm/scr/bootstrap.c) to build Stage 0 and bootstrap the self-hosted compiler.
- **Self-Hosting Verification**: `wyrmc` achieves Stage 2 self-compilation (`wyrmc_stage1.exe build compiler/wyrmc.wyr -o wyrmc.exe`).

---

## 2. Source Files & Lexical Rules

- Source file extension: `.wyr`
- Line comments start with `//`.
- Block comments use `/*` and `*/`.
- Documentation comments start with `///`.
- Block delimiters use `{` and `}`.
- Semicolons: **Optional** at most statement boundaries; **required** at the end of `use` statements (e.g. `use std.sdl;`).
- Editor Support: VS Code syntax highlighting extension with TextMate grammar lives in [extension/](file:///c:/Users/BEST/Desktop/wyrm/extension).

---

## 3. Values & Static Types (v3.1.0 Gradual / Hybrid Model)

Wyrm v3.1.0 provides **Gradual / Hybrid Static Typing** combining the ergonomics of dynamic scripting with C/Rust unboxed native CPU execution:

### Static Primitive Types
When explicitly annotated, Wyrm generates unboxed LLVM IR instructions without value boxing overhead:
- **`i32` / `int`**: Signed 32-bit integer (`alloca i32`, direct integer arithmetic `add i32`, `sub i32`, `mul i32`, `sdiv i32`, `srem i32`, `icmp`).
- **`i64`**: Signed 64-bit integer (`alloca i64`, 64-bit integer operations `add i64`, `sub i64`, `mul i64`, `sdiv i64`, `srem i64`, `icmp`).
- **`u8` / `byte`**: Unsigned 8-bit integer (`alloca i8`, byte operations and zero-extension conversions).
- **`f32`**: Single-precision 32-bit IEEE-754 floating point (`alloca float`, native float instructions `fadd`, `fsub`, `fmul`, `fdiv`, `fcmp`).
- **`f64` / `float`**: Double-precision 64-bit IEEE-754 floating point (`alloca double`, native double instructions `fadd`, `fsub`, `fmul`, `fdiv`, `fcmp`).
- **`bool`**: 1-bit boolean register (`alloca i1`).

### Dynamic Tagged Union (`Value`)
When variables are unannotated or bridged with dynamic calls, values use the reference-counted runtime union `Value`:
- **`null`**: The empty / null value (`VAL_NULL`, [val_null](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_core.c#L14)).
- **`bool`**: Boolean values `true` and `false` (`VAL_BOOL`).
- **`number`**: Double-precision floating point number (`VAL_NUMBER`).
- **`string`**: Dynamically allocated UTF-8 character string (`VAL_STRING`, `char *string;`).
- **`array`**: Dynamic heap-allocated list (`VAL_ARRAY`, `ValArray *array;`).
- **`struct`**: User-defined data structure (`VAL_STRUCT`, `WyrmStruct *structure;`) with named fields, methods, and deterministic reference counting.
- **`raw_ptr`**: Raw pointer (`VAL_RAW_PTR`, `void *raw_ptr;`) for FFI handles, arena buffers, and native objects.

---

## 4. Variables & Binding

- **`var` (Mutable with Optional Static Typing)**:
  ```wyrm
  var score: i32 = 100       // Unboxed static i32 (direct CPU register arithmetic)
  var name = "Wyrm"         // Dynamic type inference
  score = score + 50
  ```
- **Implicit Assignment**: If a variable has not been declared with `var`, assigning to an identifier declares it in the local scope or updates an existing outer variable:
  ```wyrm
  x = 10
  ```
- **`dec` (Immutable)**: Declares a constant binding that cannot be reassigned:
  ```wyrm
  dec PI = 3.1415926535
  ```

---

## 5. Operators

- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `//` (floor division), `**` (power)
- **Comparison**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical**: `&&`, `||`, `!` (along with keyword aliases `and`, `or`, `not`)

---

## 6. Functions & Entry Point

Functions are declared with the `fn` keyword. Functions can include optional static parameter types and return types (`: Type` or `-> Type`). The `fn main()` function serves as the program entry point and is automatically executed on startup:

```wyrm
fn add(a: i32, b: i32): i32 {
    return a + b
}

fn main() {
    var result: i32 = add(5, 10)
    print("Result:", result)
}
```

Recursion depth is protected by a call-stack guard ([MAX_CALL_DEPTH](file:///c:/Users/BEST/Desktop/wyrm/compiler/interpreter/interpreter.hpp#L65) = 500) to prevent stack overflows.

---

## 7. Control Flow

### Conditional Statements
Uses `if`, `elif`, and `else` with `{}` block syntax:
```wyrm
if x > 10 {
    print("high")
} elif x > 5 {
    print("medium")
} else {
    print("low")
}
```

### Loops
The primary loop structure is **`do { ... } til <condition>`** (with `repeat { ... } til` accepted as an alias). The loop body executes repeatedly until the `<condition>` becomes `true`:
```wyrm
var i = 0
do {
    print(i)
    i = i + 1
} til i >= 5
```
Loops support `break` and `continue` statements.

---

## 8. Structs & Methods

Wyrm provides systems-style data structures with named fields, methods, and automatic memory lifecycle management:

### Struct Definition & Method Declarations
```wyrm
struct Point {
    x: i32,
    y: i32,

    fn distance_sq(self) {
        return self.x * self.x + self.y * self.y
    }

    fn translate(self, dx, dy) {
        self.x = self.x + dx
        self.y = self.y + dy
    }
}
```

### Instantiation, Field Access & Mutation
- **Construction**: Call the struct name like a function: `var p = Point(10, 20)`
- **Member Access**: Read fields with dot syntax: `p.x`, `p.y`
- **Member Mutation**: Modify fields in place: `p.x = 30`
- **Method Calls**: Invoke methods using `obj.method(args...)`. The receiver object is automatically passed as `self`.
- **In-Place Self Mutation**: Methods modifying `self.field = value` mutate the struct instance in place.

### Memory & Lifecycle
Structs are managed via deterministic reference counting in [WyrmStruct](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_core.h). Passing a struct into a method or binding to a variable increments its reference count; scope exit decrements it. When the count reaches zero, all fields and heap-allocated resources are released recursively via [val_drop](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_core.c).

---

## 9. Data Structures & Subscripting

- **Array Literals**: `var list = [10, 20, 30]`
- **Numeric Indexing**: `list[0]` (0-indexed, supports negative indices like `list[-1]`)
- **Multi-Dimensional Index Assignment**: `grid[x][y] = value`
- **Array Slicing**: `list[1:3]` (returns an independent deep-copy slice)
- **Dictionary / Map Subscripting**: JSON objects and HashMaps support string key subscripting:
  ```wyrm
  var item = json_parse("{\"title\": \"Game\"}")
  print(item["title"])
  item["title"] = "Updated Game"
  ```

---

## 10. Memory Management (Deterministic Reference Counting, RAII Drop Glue & Arena Allocators)

Wyrm employs a **Deterministic Dynamic Value Runtime with RAII Drop Glue and Scoped Arena Allocators**:

1. **Deterministic Drop Glue ([val_drop](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_core.c#L525))**:
   - Scope-bound RAII cleans up heap-allocated strings, arrays, and reference-counted structs when variables exit scope in [Environment](file:///c:/Users/BEST/Desktop/wyrm/compiler/interpreter/environment.hpp#L32).
   - Reassigning variables or overwriting array elements automatically drops the previous value to prevent memory leaks.
2. **Reference Counting for Structs**:
   - `WyrmStruct` instances track active references (`ref_count`). Passing a struct as `self` or copying a reference increments the count; exiting a scope decrements it. When `ref_count <= 0`, all inner fields are dropped recursively and the struct header is freed.
3. **Deep Copy ([val_copy](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_core.c#L554))**:
   - Explicit independent cloning for string and array data, or reference count increments for structs.
4. **Arena Allocation ([WyrmArena](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_arena.h#L7))**:
   - High-throughput region memory allocation released all at once via [arena_destroy](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_arena.c#L47) or reset via [arena_reset](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_arena.c#L41):
   ```wyrm
   arena buf(1024)
   var p = buf.alloc(64)
   buf.reset()
   ```
5. **Scoped Ownership Drop Glue**:
   - Blocks and loops automatically release scoped bindings and child struct instances when control flow exits the scope.
6. **Unsafe Blocks**:
   - Low-level manual memory allocation using `malloc`, `realloc`, and `free` is guarded within `unsafe { ... }` blocks:
   ```wyrm
   unsafe {
       var p = malloc(64)
       free(p)
   }
   ```

---

## 11. Standard Library Modules (v3.1.0)

Imported via `use std.<module>;`:

### `std.sdl` - Windowing & Game Event Loop
Hardware-accelerated 2D graphics, windowing, and input handling powered by dynamic SDL2 runtime binding:
```wyrm
use std.sdl;

fn main() {
    sdl_init()
    var win = sdl_window("Wyrm Game", 800, 600)
    var running = true

    do {
        var ev = sdl_poll_event()
        if ev["type"] == SDL_QUIT {
            running = false
        }
        if ev["type"] == SDL_KEYDOWN {
            print("Key pressed:", ev["key"])
        }

        sdl_clear(win, 0, 0, 0)
        sdl_present(win)
        sdl_delay(16)
    } til running == false

    sdl_quit()
}
```

### `std.ffi` - Foreign Function Interface
Load native shared libraries (`.dll`, `.so`, `.dylib`) dynamically:
```wyrm
use std.ffi;

fn main() {
    var lib = ffi_open("my_native_lib.dll")
    var fn_sym = ffi_sym(lib, "calculate")
    var res = ffi_call(fn_sym, [10, 20])
    ffi_close(lib)
}
```

### `std.json` - JSON Serialization
RFC 8259 compliant single-pass parser and encoder:
```wyrm
use std.json;

fn main() {
    var obj = json_parse("{\"name\": \"Wyrm\", \"version\": 2.6}")
    print(obj["name"])
    print(json_encode(obj))
    print(json_pretty(obj, 2))
}
```

### `std.yaml` - YAML Serialization
Block-style YAML parser supporting scalars, sequences, and mappings:
```wyrm
use std.yaml;

fn main() {
    var cfg = yaml_parse("host: localhost\nport: 8080\n")
    print(cfg["host"])
}
```

### `std.thread` - Multithreading & Synchronization
Spawns isolated OS-level worker threads with mutex primitives:
```wyrm
use std.thread;

fn worker(id) {
    print("Worker running:", id)
}

fn main() {
    var t = thread_spawn(worker, [1])
    thread_join(t)

    var m = mutex_new()
    mutex_lock(m)
    // critical section
    mutex_unlock(m)
    mutex_destroy(m)
}
```

### `std.collections` - High-Performance Collections
Open-addressing FNV-1a HashMaps and Set collections:
```wyrm
use std.collections;

fn main() {
    var m = map_new()
    map_set(m, "player", "WyrmMaster")
    print(m["player"])
    print(map_len(m))
}
```

---

## 12. Built-in Core Functions

Wyrm provides the following core built-in operations:

- `print(...)`: Prints arguments separated by space to stdout.
- `input(prompt)`: Reads a line of input from stdin.
- `len(obj)`: Returns the length of an array or string.
- `append(arr, item)`: Appends an item to a dynamic array.
- `pop(arr)`: Removes and returns the last element of an array.
- `int(v)` / `float(v)`: Robust numerical conversion via `strtod`.
- `str(v)`: String conversion.
- `split(s, sep)` / `join(sep, arr)`: String splitting and joining.
- `trim(s)` / `upper(s)` / `lower(s)`: String formatting.
- `contains(s, sub)` / `replace(s, old, new)`: Substring searching and replacement.
- `starts_with(s, prefix)` / `ends_with(s, suffix)`: Affix matching.
- `char_at(s, idx)`: Character extraction.
- `ord_val(c)` / `chr_val(n)`: Character code conversions.
- `to_bytes(s)` / `from_bytes(arr)`: Byte array conversions.
- `read_file(path)` / `write_file(path, content)`: Safe file I/O.

---

## 13. Package Manager & Public Registry (`wyrpkg`)

`wyrpkg` manages Wyrm projects and supports a distributed, Git-based Public Package Registry:

```bash
# Create or initialize projects
wyrpkg new my_game
wyrpkg init

# Build or run
wyrpkg build
wyrpkg run

# Install packages from Public Registry (GitHub, Git remote, or local directory)
wyrpkg install owner/repo
wyrpkg install https://github.com/owner/repo.git
wyrpkg install ./local_library

# Update installed Git packages
wyrpkg update repo

# Validate and publish release instructions
wyrpkg publish

# List and remove packages
wyrpkg list
wyrpkg remove repo
```

---

## 14. Native Bootstrap Build & Installation

To build and install the Wyrm toolchain across platforms:

### Linux & macOS (Bash)
```bash
chmod +x install.sh
./install.sh
```

### Windows (PowerShell)
```powershell
powershell -File install.ps1
```

### Windows C11 Self-Contained Bootstrap Driver
```bash
gcc wyrm/scr/bootstrap.c -o bootstrap.exe -std=c11 -O2
./bootstrap.exe
```

This compiles `wyrmc` and `wyrpkg`, deploys the standard library packages, and configures user environment paths automatically.

---

## 15. Compiler Visual Diagnostics (v3.1.0)

Wyrm v3.1.0 features compiler diagnostics styled after Rust and Clang, providing source code context, column markers, and standard error classification codes:

### Visual Error Formatting
When a syntax, semantic, or lexical error is encountered, `wyrmc` highlights the exact offending code line and column:

```text
error[E0002]: syntax error: unexpected token NEWLINE '
'
  --> examples/test_diagnostics_demo.wyr:3:21
   |
 3 |     var result = a +
   |                     ^ expression expected
   |
```

### Diagnostic Error Classification Codes
- **`E0001`**: Lexical analysis error (unrecognized characters, unterminated string or comment literals)
- **`E0002`**: Parser syntax error (unexpected tokens, missing delimiters, malformed statements)
- **`E0003`**: Semantic analysis error (type mismatches, undeclared identifiers, invalid assignments)
- **`E0004`**: Codegen and backend compilation failure (native linking issues, unsupported target flags)

