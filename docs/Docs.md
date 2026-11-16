# Wyrm Language Specification

This document describes the Wyrm language design and architecture. The canonical project version is stored in the [VERSION](file:///c:/Users/BEST/Desktop/wyrm/VERSION) file (currently **1.2.0**).

---

## 1. Toolchain & Runtime

- **`wyrmc`**: The primary compiler and execution runner. Supports:
  - `wyrmc build <file.wyr>` - Ahead-of-time native binary compilation via Transpiler to C11 or LLVM IR -> Clang.
  - `wyrmc run <file.wyr>` - Instant interpreter mode (C++ tree-walking interpreter).
  - `wyrmc test [dir]` - Discover and run all `test_*.wyr` files, report PASS/FAIL.
  - `wyrmc check <file.wyr>` - Parse and lint only, no execution.
  - `wyrmc fmt <file.wyr>` - Format source file in-place.
  - `wyrmc clean [--all]` - Remove build artifacts and cached binaries.
- **`wyrpkg`**: Project and package manager (Cargo-inspired): `new`, `init`, `build`, `run`, `install`, `add`, `lock`, `update`, `publish`, `remove`, `list`.
- **AOT Codegen Path**: Transpiler -> C11 -> gcc, or LLVM IR -> Clang / LLVM.
- **Self-Hosting**: `wyrmc` is self-hosted in pure Wyrm (`compiler/wyrmc.wyr`), achieving Stage 2 self-compilation.
- **Stage 0 Bootstrap**: C11 / C++20 bootstrap (`wyrm/src/bootstrap.c`) used exclusively to bootstrap Stage 0.

---

## 2. Source Files & Lexical Rules

- Source file extension: `.wyr`
- Line comments: `//`
- Block comments: `/* */`
- Documentation comments: `///`
- Block delimiters: `{ }`
- Semicolons: **Optional** at most statement boundaries; **required** after `use` statements (e.g. `use std.time;`).
- Editor support: VS Code syntax highlighting extension in [extension/](file:///c:/Users/BEST/Desktop/wyrm/extension).

---

## 3. Values & Static Types (Gradual / Hybrid Model)

Wyrm provides **Gradual / Hybrid Static Typing** combining unboxed native CPU execution for annotated code with a dynamic tagged union for unannotated code.

### Static Primitive Types

When explicitly annotated, Wyrm generates unboxed LLVM IR without value boxing overhead:

| Type | Alias | LLVM IR | Description |
|---|---|---|---|
| `i32` | `int` | `alloca i32` | Signed 32-bit integer |
| `i64` | - | `alloca i64` | Signed 64-bit integer |
| `u8` | `byte` | `alloca i8` | Unsigned 8-bit integer |
| `f32` | - | `alloca float` | Single-precision 32-bit float |
| `f64` | `float` | `alloca double` | Double-precision 64-bit float |
| `bool` | - | `alloca i1` | Boolean (1-bit register) |

### Dynamic Tagged Union (`Value`)

Unannotated or bridge-mode values use the reference-counted runtime union:

- `null` - Empty / null value (`VAL_NULL`).
- `bool` - Boolean `true` / `false` (`VAL_BOOL`).
- `number` - IEEE-754 64-bit double (`VAL_NUMBER`).
- `string` - Heap-allocated UTF-8 string (`VAL_STRING`, `char*`).
- `array` - Dynamic heap list (`VAL_ARRAY`, `ValArray*`).
- `struct` - User-defined data structure with named fields and reference counting (`VAL_STRUCT`, `WyrmStruct*`).
- `raw_ptr` - Raw native pointer (`VAL_RAW_PTR`, `void*`) for FFI handles and arena buffers.
- `error` - Runtime error value (`VAL_ERROR`).
- `weak_ref` - Non-owning weak reference to a struct (`VAL_WEAK_REF`).

---

## 4. Variables & Binding

- **`var` (Mutable)**:
  ```wyrm
  var score: i32 = 100       // Unboxed static i32
  var name = "Wyrm"          // Dynamic inference
  score = score + 50
  ```
- **Implicit Assignment**: Assigning to an undeclared identifier declares it in local scope or updates an outer binding:
  ```wyrm
  x = 10
  ```
- **`dec` (Immutable constant)**:
  ```wyrm
  dec PI = 3.14159265358979
  ```
- **`owned` (Ownership declaration)**:
  ```wyrm
  owned res: Resource = Resource(id, tag)
  ```
  An `owned` binding transfers heap resources out of scope when returned or moved. The environment marks the slot as moved and skips drop on exit.

---

## 5. Operators

- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `//` (floor division), `**` (power)
- **Comparison**: `==`, `!=`, `<`, `>`, `<=`, `>=`
- **Logical**: `&&`, `||`, `!` (aliases: `and`, `or`, `not`)

---

## 6. Functions & Entry Point

```wyrm
fn add(a: i32, b: i32): i32 {
    return a + b
}

fn main() {
    var result: i32 = add(5, 10)
    print("Result:", result)
}
```

- Optional type annotations: `: Type` or `-> Type` for return types.
- `fn main()` is the program entry point, called automatically.
- Recursion depth guard: [MAX_CALL_DEPTH](file:///c:/Users/BEST/Desktop/wyrm/compiler/interpreter/interpreter.hpp#L65) = 500.

### Generic Functions (1.2.0)

Type parameters are erased at runtime (all map to `Value`). Syntax:
```wyrm
fn swap<T>(a: T, b: T) {
    var tmp = a
    a = b
    b = tmp
}
```

### Async Functions (1.2.0)

`async fn` marks a function as asynchronous. `await` evaluates the expression immediately in the current sequential execution model:
```wyrm
async fn fetch_data(url) {
    var result = await http_get(url)
    return result
}
```

### Closures / Lambda (1.2.0)

Anonymous functions are first-class values:
```wyrm
var double = fn(x) { return x * 2 }
var nums = [1, 2, 3, 4]
var i = 0
do {
    print(double(nums[i]))
    i = i + 1
} til i >= len(nums)
```

---

## 7. Control Flow

### Conditional Statements

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

```wyrm
var i = 0
do {
    print(i)
    i = i + 1
} til i >= 5
```

`break` and `continue` are supported inside `do...til` loops.

---

## 8. Structs & Methods

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

var p = Point(3, 4)
p.translate(10, 20)
print(p.distance_sq())   // 745
```

### Generic Structs (1.2.0)

```wyrm
struct Box<T> {
    value: T,
}
var b = Box(42)
```

### Memory & Lifecycle

`WyrmStruct` instances are managed by deterministic reference counting (`ref_count`). When `ref_count <= 0` all fields are dropped recursively via [val_drop](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_core.c).

---

## 9. Enums & Pattern Matching (1.2.0)

### Enum Definition

```wyrm
enum Color {
    Red,
    Green,
    Blue,
}

enum Shape {
    Circle(radius),
    Rect(width, height),
    Point,
}
```

### Match Statement

```wyrm
var s = Shape.Rect(10, 5)
match s {
    case Circle(r) -> print("Circle, radius:", r)
    case Rect(w, h) -> print("Area:", w * h)
    case Point -> print("A point")
    default -> print("Unknown")
}
```

---

## 10. Error Handling (1.2.0)

### try / catch / throw

```wyrm
fn read_config(path) {
    if !fs_exists(path) {
        throw "Config file not found: " + path
    }
    return fs_read(path)
}

fn main() {
    try {
        var cfg = read_config("config.yaml")
        print("Loaded:", cfg)
    } catch e {
        print("Error:", e)
    }
}
```

### Result Type Helpers

The C runtime provides `val_ok`, `val_err`, `val_is_ok`, `val_is_err`, and `val_unwrap` for explicit Result-style error propagation from C stdlib functions.

---

## 11. Memory Management

Wyrm employs **Deterministic Drop Glue + Scoped Arena Allocators + Ownership Move Semantics**:

1. **RAII Drop Glue ([val_drop](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_core.c))**: Heap strings, arrays, and reference-counted structs are freed when variables exit scope via the [Environment](file:///c:/Users/BEST/Desktop/wyrm/compiler/interpreter/environment.hpp) destructor. Reassignments drop old values immediately.

2. **Reference Counting for Structs**: `WyrmStruct` tracks `ref_count`. Copies via [val_copy](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_core.c) increment it; scope exit decrements. At zero, inner fields are dropped recursively and the struct header freed.

3. **Ownership (`owned`) & Move Semantics (1.1.0+)**:
   ```wyrm
   owned res: Resource = create_resource(1, "primary")
   ```
   - Returning an `owned` variable calls `mark_moved` in the Environment, transferring the heap resources to the caller without double-free.
   - The slot is nulled and flagged `is_moved`; the destructor skips it.

4. **Arena Allocators ([WyrmArena](file:///c:/Users/BEST/Desktop/wyrm/wyrm/lib/wyrm_arena.h))**:
   ```wyrm
   arena buf(4096)
   var p = buf.alloc(64)
   buf.reset()              // reclaim all at once
   ```

5. **Unsafe Blocks**:
   ```wyrm
   unsafe {
       var p = malloc(64)
       // pointer arithmetic
       free(p)
   }
   ```

6. **Weak References**:
   ```wyrm
   var weak = weak_ref(strong_struct)
   var locked = lock(weak)    // returns null if target was freed
   ```

---

## 12. Data Structures & Subscripting

- **Array Literals**: `var list = [10, 20, 30]`
- **Indexing**: `list[0]`, negative indices `list[-1]`.
- **Slicing**: `list[start:end]` - half-open, returns a deep copy.
- **Dictionary / JSON subscripting**: `obj["key"]`, `obj["key"] = value`.
- **Built-in array/string operations**: `join`, `split`, `len`, `append`, `pop`.

---

## 13. Standard Library Modules (Dual-Aliasing)

Use with `use <alias>;` or `use std.<name>;`.

### `std.os` - Operating System (1.2.0)

```wyrm
use std.os;

fn main() {
    var args = os_args()
    var home = os_env("HOME")
    os_setenv("APP_ENV", "production")
    var cwd = os_cwd()
    os_chdir("/tmp")
    var platform = os_platform()    // "windows", "linux", or "macos"
    var sep = os_sep()              // "\" or "/"
    var env_map = os_environ()
    os_exec("echo hello")
    os_exit(0)
}
```

### `std.fs` - File System (1.2.0)

```wyrm
use std.fs;

fn main() {
    var content = fs_read("data.txt")
    fs_write("out.txt", "Hello, Wyrm!")
    fs_append("log.txt", "line\n")
    var exists = fs_exists("config.yaml")
    fs_delete("temp.tmp")
    fs_mkdir("build/output")
    var entries = fs_readdir("src/")
    var info = fs_stat("main.wyr")     // struct: size, is_dir, mtime
}
```

### `std.path` - Path Utilities (1.2.0)

```wyrm
use std.path;

fn main() {
    var full = path_join("src", "main.wyr")
    var base = path_basename("/usr/lib/libwyrm.so")   // "libwyrm.so"
    var dir  = path_dirname("/usr/lib/libwyrm.so")    // "/usr/lib"
    var ext  = path_ext("report.md")                   // ".md"
    var abs  = path_abs("../config.yaml")
    var is_abs = path_is_abs("/usr/bin")               // true
}
```

### `std.io` - I/O Streams (1.2.0)

```wyrm
use std.io;

fn main() {
    var line = io_readline()          // read from stdin
    io_write("Enter name: ")
    io_flush()
    io_stderr("Fatal: bad input\n")
}
```

### `std.http` - HTTP Client (1.2.0)

```wyrm
use std.http;

fn main() {
    var res = http_get("https://api.example.com/data")
    print(res["status"])     // 200
    print(res["body"])

    var headers = http_headers(["Authorization: Bearer token"])
    var post = http_post("https://api.example.com/create",
                         "{\"name\": \"wyrm\"}", headers)
    print(post["status"])
}
```

### `std.time` - Timers & Formatting

```wyrm
use std.time;

fn main() {
    var now_sec  = time_now()
    var unix_sec = time_unix()
    var unix_ms  = time_unix_ms()
    var t0 = time_monotonic_ms()
    time_sleep(25)
    var elapsed = time_diff(t0, time_monotonic_ms())
    var utc  = time_format(unix_sec, "%Y-%m-%d")
    var local = time_format_local(unix_sec, "%Y-%m-%d %H:%M:%S")
}
```

### `std.random` - Random Number Generation

```wyrm
use std.random;

fn main() {
    rand_seed(12345)
    var roll   = rand_int(1, 20)
    var item   = rand_choice(["sword", "shield", "potion"])
    var deck   = rand_shuffle([1, 2, 3, 4, 5])
    var token  = rand_bytes_hex(32)    // CSPRNG
    var hw_val = rand_trng()           // TRNG / RDRAND
}
```

### `std.json` - JSON

```wyrm
use std.json;

fn main() {
    var obj = json_parse("{\"name\": \"Wyrm\", \"version\": 2.6}")
    print(obj["name"])
    print(json_encode(obj))
    print(json_pretty(obj, 2))
}
```

### `std.yaml` - YAML

```wyrm
use std.yaml;

fn main() {
    var cfg = yaml_parse("host: localhost\nport: 8080\n")
    print(cfg["host"])
}
```

### `std.collections` - HashMap & Set

```wyrm
use std.collections;

fn main() {
    var m = map_new()
    map_set(m, "player", "WyrmMaster")
    print(map_get(m, "player"))
    print(map_has(m, "player"))
    print(map_len(m))
    var keys = map_keys(m)
    map_delete(m, "player")

    var s = set_new()
    set_add(s, "item")
    print(set_has(s, "item"))
    var u = set_union(s, s)
}
```

### `std.ffi` - Foreign Function Interface

```wyrm
use std.ffi;

fn main() {
    var lib = ffi_open("my_native_lib.dll")
    var fn_sym = ffi_sym(lib, "calculate")
    var res = ffi_call(fn_sym, [10, 20])
    ffi_close(lib)
}
```

### `std.thread` - OS Threads

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
    mutex_unlock(m)
    mutex_destroy(m)
}
```

---

## 14. Pure Wyrm Packages (`library/std/`)

Imported with a relative path: `use library.std.math;`

### `std.math`

Mathematical constants and functions written entirely in Wyrm:

```wyrm
use library.std.math;

fn main() {
    print(MATH_PI)
    print(math_sqrt(2.0))
    print(math_sin(math_radians(90.0)))
    print(math_log10(1000.0))
    print(math_clamp(1.5, 0.0, 1.0))
    print(math_lerp(0.0, 100.0, 0.25))
    print(math_is_prime(97.0))
    var stats = [10.0, 20.0, 30.0, 40.0]
    print(math_mean(stats))
    print(math_gcd(48.0, 18.0))
}
```

**Available functions**: `math_floor`, `math_ceil`, `math_round`, `math_trunc`, `math_abs`, `math_sign`, `math_pow`, `math_sqrt`, `math_cbrt`, `math_log`, `math_log10`, `math_log2`, `math_exp`, `math_sin`, `math_cos`, `math_tan`, `math_sinh`, `math_cosh`, `math_tanh`, `math_radians`, `math_degrees`, `math_min`, `math_max`, `math_clamp`, `math_lerp`, `math_normalize`, `math_remap`, `math_fmod`, `math_divmod`, `math_gcd`, `math_lcm`, `math_is_prime`, `math_sum`, `math_mean`, `math_array_min`, `math_array_max`, `math_approx_eq`.

**Constants**: `MATH_PI`, `MATH_TAU`, `MATH_E`, `MATH_LN2`, `MATH_LN10`, `MATH_INF`.

### `std.string`

String inspection, transformation, formatting, and builder utilities:

```wyrm
use library.std.string;

fn main() {
    print(str_pad_left("42", 6, "0"))      // "000042"
    print(str_capitalize("hello world"))    // "Hello world"
    print(str_title("the quick fox"))       // "The Quick Fox"
    print(str_repeat("ab", 3))             // "ababab"
    print(str_index_of("hello", "ll"))     // 2
    print(str_count("abcabc", "bc"))       // 2

    var b = str_builder_new()
    b = str_builder_push(b, "Hello, ")
    b = str_builder_pushln(b, "Wyrm!")
    print(str_builder_build(b))

    var lines = str_split_lines("line1\nline2\nline3")
    print(len(lines))                      // 3
}
```

**Available functions**: `str_len`, `str_empty`, `str_starts_with`, `str_ends_with`, `str_contains`, `str_char_at`, `str_ord`, `str_chr`, `str_to_bytes`, `str_from_bytes`, `str_upper`, `str_lower`, `str_capitalize`, `str_title`, `str_trim`, `str_trim_left`, `str_trim_right`, `str_pad_left`, `str_pad_right`, `str_pad_center`, `str_slice`, `str_take`, `str_drop`, `str_split`, `str_chars`, `str_split_lines`, `str_join`, `str_replace`, `str_index_of`, `str_last_index_of`, `str_count`, `str_from`, `str_to_number`, `str_is_integer`, `str_is_number`, `str_repeat`, `str_fill`, `str_builder_new`, `str_builder_push`, `str_builder_pushln`, `str_builder_build`, `str_builder_len`.

### `std.log`

Structured, leveled logging with optional timestamps and ANSI color output:

```wyrm
use library.std.log;

fn main() {
    log_init("MyApp", LOG_LEVEL_DEBUG)
    log_set_timestamps(true)
    log_set_color(true)

    log_debug("Parsing configuration...")
    log_info("Server started on port 8080")
    log_warn("High memory usage detected")
    log_error("Failed to connect to database")

    log_field("Request", "method", "GET")
    log_fields2("Response", "status", 200, "size", 4096)

    log_section("Database")
    log_info("Running migration v3")
    log_section_end("Database")

    var t0 = log_bench_start("heavy computation")
    // ... work ...
    log_bench_end("heavy computation", t0)

    log_progress("Uploading", 75, 100)
}
```

**Log levels**: `LOG_LEVEL_DEBUG` (0), `LOG_LEVEL_INFO` (1), `LOG_LEVEL_WARN` (2), `LOG_LEVEL_ERROR` (3), `LOG_LEVEL_FATAL` (4), `LOG_LEVEL_OFF` (5).

**Fatal logging terminates the process** via `os_exit(1)`.

---

## 15. Built-in Core Functions

| Function | Description |
|---|---|
| `print(...)` | Print arguments separated by space to stdout |
| `input(prompt)` | Read a line from stdin |
| `len(obj)` | Length of array or string |
| `append(arr, item)` | Append item to dynamic array |
| `pop(arr)` | Remove and return last array element |
| `int(v)` / `float(v)` | Numeric conversion via `strtod` |
| `str(v)` | String conversion |
| `split(s, sep)` / `join(sep, arr)` | String split and join |
| `trim(s)` / `upper(s)` / `lower(s)` | String formatting |
| `contains(s, sub)` / `replace(s, old, new)` | Substring operations |
| `starts_with(s, p)` / `ends_with(s, s)` | Affix matching |
| `char_at(s, i)` | Character extraction |
| `ord_val(c)` / `chr_val(n)` | Character code conversions |
| `to_bytes(s)` / `from_bytes(arr)` | UTF-8 byte array conversions |
| `read_file(path)` / `write_file(path, content)` | Legacy file I/O |

---

## 16. Package Manager (`wyrpkg`)

```bash
wyrpkg new my_game
wyrpkg init
wyrpkg build
wyrpkg run
wyrpkg install owner/repo
wyrpkg install ./local_library
wyrpkg add std_time@1.2.0
wyrpkg lock
wyrpkg update repo
wyrpkg publish
wyrpkg list
wyrpkg remove repo
```

### Manifest (`wyrpkg.toml`)

```toml
[package]
name = "my_game"
version = "0.1.0"
entry = "main.wyr"

[dependencies]
# std_time = "1.2.0"
```

---

## 17. Native Bootstrap & Installation

### Linux & macOS

```bash
chmod +x install.sh
./install.sh
```

### Windows

```powershell
powershell -File install.ps1
```

### Self-Contained Bootstrap

```bash
gcc wyrm/src/bootstrap.c -o bootstrap.exe -std=c11 -O2
./bootstrap.exe
```

Compiles `wyrmc` and `wyrpkg`, deploys the C runtime library packages, and configures user `PATH` automatically.

---

## 18. Compiler Diagnostics

Wyrm diagnostics are styled after Rust and Clang, providing source context, column markers, and error classification codes:

```text
error[E0002]: syntax error: unexpected token NEWLINE
  --> examples/test_diagnostics_demo.wyr:3:21
   |
 3 |     var result = a +
   |                     ^ expression expected
   |
```

### Error Codes

| Code | Category |
|---|---|
| `E0001` | Lexical error (unrecognized characters, unterminated strings/comments) |
| `E0002` | Parser syntax error (unexpected tokens, missing delimiters) |
| `E0003` | Semantic error (type mismatches, undeclared identifiers) |
| `E0004` | Codegen / backend failure (linking errors, unsupported target flags) |
| `E0010` | Deprecated / removed syntax (e.g. legacy `repeat` loop keyword) |
