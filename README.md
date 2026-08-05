# Wyrm Language
[![License: MIT](https://img.shields.io/badge/License-MIT-333333.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-2.2.0-B10C1A)](https://github.com/neofilisoft/wyrm/releases)

Wyrm (`.wyr`) is a static systems programming language. The main toolchain consists of `wyrmc` (the compiler & runner) and `wyrpkg` (the package manager). The Python package remains in the repository as a development interpreter, parser test bed, and browser compiler source.

## Position

- Static systems language
- AOT-first compilation strategy
- Primary codegen and runtime path: C11
- Native tools: `wyrmc.exe` and `wyrpkg.exe`
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
- Module imports: `use module.wyr` or `use module.wyr;`
- Comments: `//`, `/* */`, `///`
- Optional semicolons
- Ownership and RAII direction with `owned`, `unsafe`, `arena` allocation, and raw memory APIs (`malloc`, `free`, `realloc`)
- 14 built-in String & Data manipulation operations: `split`, `join`, `trim`, `upper`, `lower`, `contains`, `replace`, `starts_with`, `ends_with`, `char_at`, `ord_val`, `chr_val`, `to_bytes`, `from_bytes`

## Native Toolchain & Bootstrap

Build the toolchain from source using the self-contained C11 bootstrap compiler:

```bash
gcc wyrm/scr/bootstrap.c -o bootstrap.exe -std=c11 -O2
./bootstrap.exe
```

The bootstrap driver will compile `wyrmc.exe` and `wyrpkg.exe` to your `~/.wyrm` directory, copy the Python runtime packages to `~/.wyrm/packages/wyrmlang/`, and add the installation folder to your User `PATH`.

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

- `wyrm/scr/`: native sources for the bootstrap compiler (`bootstrap.c`), `wyrmc.cpp`, and `wyrpkg.c`
- `wyrm/lib/`: standard C headers (`wyrm_core.h`, `wyrm_str.h`, `wyrm_arena.h`) linked by compiled programs
- `wyrm/`: Python development interpreter, parser, and browser compiler source
- `examples/`: runnable Wyrm programs
- `docs/Docs.md`: language specification
- `docs/github-linguist/README.md`: suggested GitHub Linguist language entry

## Language Specification

See `docs/Docs.md` for the current Wyrm language specification.

## License

MIT License - see LICENSE.

## Copyright

Copyright (c) 2026 Neofilisoft
