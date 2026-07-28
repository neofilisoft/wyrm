# Wyrm Language
[![License: MIT](https://img.shields.io/badge/License-MIT-333333.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-2.2.0-B10C1A)](https://github.com/neofilisoft/wyrm/releases)

Wyrm (`.wyr`) is a static systems programming language. The primary execution path is AOT compilation to native binaries through a C11 codegen/runtime path. Interpreter mode exists as a secondary developer workflow for fast feedback while the language moves toward full self-hosting.

## Position

- Static systems language
- AOT-first compilation strategy
- Primary codegen target: C11
- Native runtime owned by Wyrm, developed in C11 first and intended to become self-hosted over time
- Dev interpreter mode for quick script/testing workflows
- `wyrpkg` all-in-one toolchain in the style of Cargo

## Features

- Function declarations with `fn`
- Static primitive type direction: `i8`, `i16`, `i32`, `i64`, `u8`, `u16`, `u32`, `u64`, `f32`, `f64`, `bool`, `char`, `string`
- Boolean values: `true` / `false`
- Empty value: `null`
- Logical operators: `&&`, `||`, `!` with aliases `and`, `or`, `not`
- Equality and comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Conditional statements: `if` / `elif` / `else`
- Block syntax: `{ }`
- Loops: `do` / `til` with `repeat` / `til` alias
- Arrays: `[1, 2, 3]`, indexing, slicing, and index assignment
- Module import system: `use module.wyr` or `use module.wyr;`
- Package manager/toolchain: `wyrpkg`
- Comments: `//`, `/* */`, `///`
- Optional semicolons
- Ownership and RAII direction with `owned`, `unsafe`, arena allocation, and raw memory APIs

## Native Toolchain

Build the C-native tools:

```bash
gcc scr/wyrmc.c -o wyrm.exe -std=c11 -O2 -lm
gcc scr/wyrmc.c -o wyrmc.exe -std=c11 -O2 -lm
gcc scr/wyrpkg.c -o wyrpkg.exe -std=c11 -O2
```

Run a Wyrm program in dev/interpreted mode:

```bash
./wyrm.exe run examples/hello.wyr
```

Build a Wyrm program as a native binary:

```bash
./wyrm.exe build examples/hello.wyr -o hello_native.exe
./hello_native.exe
```

`wyrmc.exe` remains as a compiler alias. `wyrm.exe` is the main user-facing tool.

## Package Manager

```bash
./wyrpkg.exe install my_package
./wyrpkg.exe list
./wyrpkg.exe remove my_package
```

## Roadmap Libraries

- High-performance File I/O
- Advanced string handling
- Comprehensive math utilities
- OS abstraction layer
- OS syscalls
- Networking

## GitHub Language Detection

The repo marks `*.wyr` as Wyrm in `.gitattributes`. GitHub will display Wyrm in the Languages sidebar once Wyrm is registered in GitHub Linguist upstream. See `docs/github-linguist/README.md` for the suggested Linguist entry.

## Language Specification

See `docs/Docs.md` and `fix.txt` for the current Wyrm language direction.

## License

MIT License - see `LICENSE`.

## Copyright

Copyright (c) 2026 Neofilisoft
