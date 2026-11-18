# Wyrm Language
[![License: MIT](https://img.shields.io/badge/License-MIT-333333.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.2.1-B10C1A)](https://github.com/neofilisoft/wyrm/releases)

Wyrm (`.wyr`) is a **static systems programming language** with a self-hosted compiler, gradual static typing, and deterministic memory management. The toolchain consists of `wyrmc` (the compiler and runner) and `wyrpkg` (the package manager).

## Position

- Systems language - not a scripting language or a virtual machine host
- Static + Gradual type system: optional type annotations, zero boxing overhead for annotated primitives
- Native binary output via Transpiler -> C11 or AOT LLVM IR -> Clang
- Full self-hosted compiler (`compiler/wyrmc.wyr`) written in pure Wyrm
- Deterministic memory management: scoped RAII, reference counting, arena allocation, and ownership move semantics
- No garbage collector; no runtime VM overhead for release builds
- C11 / C++20 bootstrap toolchain (Stage 0 only)

## Features

### Core Language (1.0.0+)
- **Gradual / Hybrid Static Typing**: Annotated variables (`var count: i64 = 0`) emit unboxed LLVM IR allocas; unannotated variables use the dynamic `Value` model.
  - Static primitive types: `i32`, `i64`, `u8`, `f32`, `f64`, `bool`
  - Typed function signatures: `fn add(a: i64, b: i64): i64`
  - Typed struct fields: `struct Point { x: i32, y: i32 }`
- **Structs and Methods**: Named fields, in-place mutation, `self` receiver methods, deterministic reference counting.
- **Memory Management**: RAII drop glue, `owned` move semantics, arena allocation, `unsafe` blocks with `malloc`/`free`/`realloc`.
- Variables: `var` (mutable), `dec` (immutable), implicit assignment.
- Control flow: `if` / `elif` / `else`, `do { ... } til <cond>`, `break`, `continue`.
- Functions: `fn`, optional type annotations, recursion guard (depth 500), `fn main()` entry point.
- Built-in types: `number`, `bool`, `string`, `array`, `struct`, `raw_ptr`, `null`.

### New in 1.2.1
- **`enum` and `match`**: Algebraic data type variants with structural pattern matching:
  ```wyrm
  enum Shape {
      Circle(radius),
      Rect(width, height),
  }

  var s = Circle(5.0)
  match s {
      case Circle(r)      -> print("Circle area:", 3.14159 * r * r)
      case Rect(w, h)     -> print("Rect area:", w * h)
      default             -> print("Unknown shape")
  }
  ```
- **`try` / `catch` / `throw`** structured error handling and `Result` type:
  ```wyrm
  try {
      throw "file not found"
  } catch e {
      print("Caught:", e)
  }
  ```
- **Generics / Type Parameters**: Type-erased generic functions and structs:
  ```wyrm
  fn swap<T>(a: T, b: T) {
      var tmp = a
      a = b
      b = tmp
  }
  ```
- **Closures / Lambda**: Anonymous functions as first-class values:
  ```wyrm
  var double = fn(x) { return x * 2 }
  print(double(21))
  ```
- **`async` / `await` syntax**: Cooperative concurrency annotation (sequential evaluation):
  ```wyrm
  async fn fetch_data(url) {
      var result = await http_get(url)
      return result
  }
  ```
- **New stdlib modules**: `os`, `fs`, `path`, `io`, `http` (see Standard Library below).
- **New Pure Wyrm packages**: `std.math`, `std.string`, `std.log` (see `library/std/`).
- **`test` / `check` / `fmt`**: Available in the Stage 0 bootstrap compiler (`wyrmc_stage0`), not yet in the self-hosted `wyrmc` (see Compiler Commands).
  - `check` performs lexical and syntactic validation only; it does not perform semantic type checking.
- **`wyrpkg add`** and **`wyrpkg lock`**: Dependency pinning.
- **Memory bug fixes**: Correct ownership transfer on `return` of `owned` values; `val_struct_create` now deep-copies initial fields.

### Standard Library Modules (Dual-Aliasing)

Use `use <module>;` or `use std.<module>;` interchangeably:

| Module | Short alias | Description |
|---|---|---|
| `std.time` | `time` | Monotonic timers, Unix epoch, sleep, date formatting |
| `std.random` | `random` | PRNG (Xoshiro256**), CSPRNG, TRNG (RDRAND) |
| `std.json` | `json` | RFC 8259 JSON parser and encoder |
| `std.yaml` | `yaml` | Block-style YAML parser and encoder |
| `std.collections` | `collections` | HashMap and Set (open-addressing, FNV-1a) |
| `std.ffi` | `ffi` | Dynamic shared library loading and FFI calls |
| `std.thread` | `thread` | OS threads, worker spawn, mutex synchronization |
| `std.os` | `os` | Args, env vars, cwd, chdir, exit, exec, platform |
| `std.fs` | `fs` | File read/write/append/delete/mkdir/readdir/stat |
| `std.path` | `path` | Path join, basename, dirname, ext, abs, normalization |
| `std.io` | `io` | stdin readline, stdout/stderr write, flush |
| `std.http` | `http` | HTTP GET/POST (libcurl), headers, response struct |

### Pure Wyrm Packages (`library/std/`)

Written entirely in Wyrm, imported with `use library.std.math;` etc:

| Package | Description |
|---|---|
| `std.math` | Floor/ceil/round, sqrt, log, exp, trig, clamp, lerp, gcd, prime test, array stats |
| `std.string` | Pad/trim/slice/count/index-of, case, lines, StringBuilder, type predicates |
| `std.log` | Leveled logging (DEBUG/INFO/WARN/ERROR/FATAL), ANSI color, timestamps, scopes, bench helpers |

## Native Toolchain

Build the toolchain from source using the self-contained C11 bootstrap compiler:

```bash
gcc wyrm/src/bootstrap.c -o bootstrap.exe -std=c11 -O2
./bootstrap.exe
```

Or run the automated installers:

```powershell
# Windows (PowerShell)
powershell -File install.ps1
```

```bash
# Linux / macOS (Bash)
chmod +x install.sh
./install.sh
```

Check versions:

```bash
wyrmc --version    # wyrmc 1.2.1
wyrpkg --version   # wyrpkg 1.2.1
```

## Compiler Commands

The executable installed as `wyrmc` is the self-hosted compiler built from
`compiler/wyrmc.wyr`. It supports:

```bash
wyrmc build  <file.wyr> [-o <out.exe>]  # Compile to native binary
wyrmc run    <file.wyr> [args...]       # Run via interpreter
wyrmc clean  [--all]                    # Remove Wyrm-owned temp files and cache
wyrmc --version                         # Print version
wyrmc --help                            # Print usage
```

The Stage 0 bootstrap compiler (`wyrmc_stage0`, built from `wyrm/src/wyrmc.cpp`)
additionally implements `test`, `check` and `fmt`:

```bash
wyrmc_stage0 test  [dir]       # Discover and run test_*.wyr files
wyrmc_stage0 check <file.wyr>  # Lex + parse validation only; no type checking, no execution
wyrmc_stage0 fmt   <file.wyr>  # Format in-place; saves the original as <file>.wyr.bak
```

> **Current gap:** `test`, `check` and `fmt` are not available in the
> self-hosted `wyrmc`. `check` validates syntax only; Wyrm has no semantic
> type-checking pass. `clean --all` removes executables only when a `.wyr`
> source with the same base name is present in the working directory.

Native builds do not require libcurl by default. The HTTP module returns a
clear unavailable error unless the target has libcurl configured. Set
`WYRM_ENABLE_CURL=1` before building when native HTTP support is available.

## Package Manager (`wyrpkg`)

```bash
wyrpkg new my_project             # Create project with wyrpkg.toml
wyrpkg init                       # Initialize in current directory
wyrpkg build                      # Build project
wyrpkg run                        # Run project
wyrpkg install owner/repo         # Install from GitHub
wyrpkg install ./my_local_lib     # Install from local path
wyrpkg add    std_time@1.2.1     # Add dependency to manifest
wyrpkg lock                       # Pin all dependencies to lock file
wyrpkg update repo                # Update a dependency
wyrpkg publish                    # Publish to public registry
wyrpkg list                       # List installed packages
wyrpkg remove package_name        # Remove a package
```

### Project Manifest (`wyrpkg.toml`)

```toml
[package]
name = "my_project"
version = "1.0.0"
entry = "main.wyr"

[dependencies]
# package_name = "1.0.0"
```

## Repository Layout

```
wyrm/src/        bootstrap.c, wyrmc.cpp, wyrpkg.cpp
wyrm/lib/        C runtime: wyrm_core.c/h, wyrm_str.c/h, wyrm_arena.c/h, wyrm_ffi.c/h
wyrm/lib/stdlib/ C stdlib backends: json, yaml, collections, os, fs, path, io, http, random, time
compiler/        Self-hosted Wyrm compiler (wyrmc.wyr) + C++ bootstrap front-end
library/std/     Pure Wyrm packages: math.wyr, string.wyr, log.wyr
examples/        Runnable Wyrm programs
extension/       VS Code syntax highlighting extension
docs/Docs.md     Language specification
```

## Language Specification

See [`docs/Docs.md`](docs/Docs.md) for the full Wyrm language specification.

## License

MIT License - see [LICENSE](LICENSE).

## Copyright

Copyright (c) 2026 Neofilisoft
