# Wyrm Language
[![License: MIT](https://img.shields.io/badge/License-MIT-333333.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-3.1.0-B10C1A)](https://github.com/neofilisoft/wyrm/releases)

Wyrm (`.wyr`) is a static systems programming language with a self-hosted compiler and gradual static typing. The main toolchain consists of `wyrmc` (the self-hosted compiler & runner written in Wyrm) and `wyrpkg` (the package manager).

## Position

- Static systems language with Gradual / Hybrid type annotations
- Fully self-hosted compiler (`compiler/wyrmc.wyr`) compiling directly to native binaries via LLVM IR and Clang
- Zero boxing overhead for static primitives (`i32`, `i64`, `u8`, `f32`, `f64`, `bool` variables, arithmetic, and branches emit direct CPU register instructions)
- C11 / C++20 bootstrap toolchain used exclusively to bootstrap Stage 0
- Native tools: `wyrmc` and `wyrpkg`
- `wyrpkg` package and project tool in the style of Cargo with Git-based Public Package Registry

## Features

- **Self-Hosted Compiler (v3.1.0)**: `wyrmc` is self-hosted in pure Wyrm (`compiler/wyrmc.wyr`) and achieves Stage 2 self-compilation.
- **Gradual / Hybrid Static Typing (v3.1.0)**:
  - Annotate variables: `var count: i64 = 0`, `var byte_val: u8 = 255`, `var ratio: f32 = 3.14`, `var flag: bool = true` (emits unboxed LLVM IR allocas, direct CPU registers, and native machine instructions)
  - Supported primitive types: `i32`, `i64`, `u8`, `f32`, `f64`, `bool`
  - Annotate functions: `fn add(a: i64, b: i64): i64` (or `-> i64`)
  - Annotate structs: `struct Point { x: i32, y: i32 }`
  - Unannotated variables seamlessly infer or use the reference-counted dynamic `Value` model without breaking backward compatibility
- **Rust/Clang-Style Visual Diagnostics (v3.1.0)**:
  - Compiler diagnostics render formatted error reports with standard error codes (`error[E0001]`, `error[E0002]`), source file snippets, line and column numbers, and underline carets (`^`).
- **Ownership & Arena Allocation (v3.1.0)**:
  - High-performance memory arena allocation via `arena buf(size)`, `buf.alloc(bytes)`, and `buf.reset()`
  - Scoped resource ownership with automatic drop glue releasing nested resources upon exiting blocks and loops
- Function declarations with `fn`
- Runtime Value Types: `number` (IEEE-754 64-bit float), `bool`, `string` (UTF-8 char*), `array` (dynamic list), `struct` (reference-counted user struct), `raw_ptr` (native pointer), `null`
- **Structs & Methods**: Data structures with named fields, in-place member mutation, and receiver `self` methods backed by deterministic reference counting:
  ```wyrm
  struct Point {
      x: i32,
      y: i32

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
  print("Distance:", p.distance_sq()) // 745
  ```
- Variables: `x = 1` (implicit) or `var x = 1` (explicit, optional); `dec` for constants
- Boolean values: `true` / `false`
- Empty value: `null`
- Logical operators: `&&`, `||`, `!` with aliases `and`, `or`, `not`
- Equality and comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Conditional statements: `if` / `elif` / `else`
- Block syntax: `{ }`
- Loops: `do` / `til` (primary loop syntax) with `repeat` / `til` alias
- Arrays: literals, indexing, slicing, and index assignment (supporting map/json string keys `obj["key"]`)
- Module imports: `use module.wyr;` or `use std.X;` (semicolon required)
- Comments: `//`, `/* */`, `///`
- Semicolons are optional at most statement boundaries; `use` statements require a trailing `;`
- `fn main()` is the program entry point and is called automatically (like C)
- Ownership and RAII direction with `owned`, `unsafe`, `arena` allocation, and raw memory APIs (`malloc`, `free`, `realloc`)
- 15 built-in type conversion and string operations: `str`, `split`, `join`, `trim`, `upper`, `lower`, `contains`, `replace`, `starts_with`, `ends_with`, `char_at`, `ord_val`, `chr_val`, `to_bytes`, `from_bytes`
- **Standard Library Modules** (v3.1.0):
  - `std.sdl`: Windowing, 2D hardware rendering, keyboard & mouse event loop
  - `std.ffi`: Foreign Function Interface (dynamic shared library loading via `LoadLibrary`/`dlopen`)
  - `std.thread`: Multithreading via OS threads, worker spawning, and mutex synchronization
  - `std.json`: RFC 8259 JSON parser & encoder with object dictionary indexing
  - `std.yaml`: Block-style YAML parser & encoder
  - `std.collections`: High-performance HashMaps and Sets

## Native Toolchain & Bootstrap

Build the toolchain from source using the self-contained C11 bootstrap compiler:

```bash
gcc wyrm/scr/bootstrap.c -o bootstrap.exe -std=c11 -O2
./bootstrap.exe
```

The bootstrap driver will compile the tools into a subdirectory layout and add each dir to your User `PATH`:

```
~/.wyrm/
  wyrmc/wyrmc.exe      <- added to PATH
  wyrpkg/wyrpkg.exe    <- added to PATH
  packages/wyrmlang/   <- C runtime library files
```

Alternatively, you can run the automated installers:

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
wyrmc --version
wyrpkg --version
```

### Compile a Wyrm Program

Build a Wyrm program as a native binary:

```bash
wyrmc build examples/hello.wyr
./hello.exe
```

Run a Wyrm program directly via the VM interpreter:

```bash
wyrmc run examples/hello.wyr
```

## Package Manager (`wyrpkg`)

`wyrpkg` is a project and package manager in the style of Cargo with a Git/GitHub-based distributed Public Registry:

```bash
# Create or initialize projects
wyrpkg new my_project
wyrpkg init

# Build or run projects
wyrpkg build
wyrpkg run

# Install packages from Public Registry (GitHub, Git repo, or local path)
wyrpkg install owner/repo
wyrpkg install https://github.com/owner/repo.git
wyrpkg install ./my_local_lib

# Update installed Git packages
wyrpkg update repo

# Publish / prepare release for Public Registry
wyrpkg publish

# Manage installed packages
wyrpkg list
wyrpkg remove package_name
```

## Repository Layout

- `wyrm/scr/`: native bootstrap compiler (`bootstrap.c`), `wyrmc.cpp`, `wyrpkg.cpp`
- `wyrm/lib/`: C runtime library (`wyrm_core.c/h`, `wyrm_str.c/h`, `wyrm_arena.c/h`, `wyrm_ffi.c/h`) linked by compiled programs
- `wyrm/lib/stdlib/`: native C backends for standard library modules (`wyrm_std_sdl`, `wyrm_std_json`, `wyrm_std_yaml`, `wyrm_std_collections`)
- `compiler/`: self-hosted Wyrm compiler source (`wyrmc.wyr`) and C++ bootstrap front-end (lexer, parser, interpreter, transpiler, and `stdlib_setup`)
- `examples/`: runnable Wyrm programs
- `extension/`: VS Code syntax highlighting extension for Wyrm (`.wyr`)
- `docs/Docs.md`: language specification
- `docs/github-linguist/README.md`: suggested GitHub Linguist language entry

## Language Specification

See `docs/Docs.md` for the current Wyrm language specification.

## License

MIT License - see LICENSE.

## Copyright

Copyright (c) 2026 Neofilisoft
