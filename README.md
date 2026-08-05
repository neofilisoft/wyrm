# Wyrm Language
[![License: MIT](https://img.shields.io/badge/License-MIT-333333.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-2.3.0-B10C1A)](https://github.com/neofilisoft/wyrm/releases)

Wyrm (`.wyr`) is a static systems programming language. The main toolchain consists of `wyrmc` (the compiler & runner) and `wyrpkg` (the package manager). The Python package remains in the repository as a development interpreter, parser test bed, and browser compiler source.

## Position

- Static systems language
- AOT-first compilation strategy
- Primary codegen path: LLVM IR - native binary via Clang (v2.4+); C11 transpiler for bootstrap
- Native tools: `wyrmc` and `wyrpkg`
- Developer interpreter mode for quick feedback
- `wyrpkg` package and project tool in the style of Cargo

## Features

- Function declarations with `fn`
- Static primitive types: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `bool`, `char`, `string`
- Boolean values: `true` / `false`
- Empty value: `null`
- Logical operators: `&&`, `||`, `!` with aliases `and`, `or`, `not`
- Equality and comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Conditional statements: `if` / `elif` / `else`
- Block syntax: `{ }`
- Loops: `do` / `til` with `repeat` / `til` alias
- Arrays: literals, indexing, slicing, and index assignment
- Module imports: `use module.wyr;` (semicolon required)
- Comments: `//`, `/* */`, `///`
- Semicolons are optional at most statement boundaries; `use` statements require a trailing `;`
- Ownership and RAII direction with `owned`, `unsafe`, `arena` allocation, and raw memory APIs (`malloc`, `free`, `realloc`)
- 14 built-in String & Data manipulation operations: `split`, `join`, `trim`, `upper`, `lower`, `contains`, `replace`, `starts_with`, `ends_with`, `char_at`, `ord_val`, `chr_val`, `to_bytes`, `from_bytes`

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

Alternatively, you can run the PowerShell installer:
```powershell
powershell -File install.ps1
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

## Package Manager

```bash
wyrpkg install my_package
wyrpkg list
wyrpkg remove my_package
```

## Repository Layout

- `wyrm/scr/`: native bootstrap compiler (`bootstrap.c`), `wyrmc.cpp`, `wyrpkg.cpp`
- `wyrm/lib/`: C runtime library (`wyrm_core.c/h`, `wyrm_str.c/h`, `wyrm_arena.c/h`) linked by compiled programs
- `compiler/`: self-hosted Wyrm compiler source (`wyrmc.wyr`) and C++ bootstrap front-end (lexer, parser, interpreter, transpiler)
- `examples/`: runnable Wyrm programs
- `docs/Docs.md`: language specification
- `docs/github-linguist/README.md`: suggested GitHub Linguist language entry

## Language Specification

See `docs/Docs.md` for the current Wyrm language specification.

## License

MIT License - see LICENSE.

## Copyright

Copyright (c) 2026 Neofilisoft
