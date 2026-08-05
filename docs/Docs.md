# Wyrm Language Specification

This document describes the current Wyrm design implemented in the repository. The canonical project version is stored in the `VERSION` file.

## Toolchain

- `wyrmc` is the main user-facing tool, functioning as a compiler and runner.
- `wyrpkg` is the package and project tool.
- AOT native compilation (v2.4+) uses LLVM IR - native binary via Clang as the primary path.
- The C11 transpiler path is used during bootstrap stage.
- The Python implementation is kept for development interpreter, parser tests, and browser compiler support.

## Source Files

Wyrm source files use the `.wyr` extension.

## Lexical Rules

- Line comments start with `//`.
- Block comments use `/*` and `*/`.
- Documentation comments start with `///`.
- Semicolons are optional at most statement boundaries.
- `use` statements require a trailing semicolon: `use module.wyr;`
- Blocks use `{` and `}`.

## Values

- `null` is the empty value.
- `true` and `false` are booleans.
- Numbers are parsed as numeric values.
- Strings use quotes.
- Arrays use bracket literals such as `[1, 2, 3]`.

## Operators

- Logical operators: `&&`, `||`, `!`
- Logical aliases: `and`, `or`, `not`
- Equality: `==`, `!=`
- Comparison: `<`, `>`, `<=`, `>=`
- Arithmetic: `+`, `-`, `*`, `/`, `%`

## Functions

Functions are declared with `fn`.

```wyrm
fn add(a, b) {
    return a + b
}

fn main() {
    print(add(5, 10))
}
```

## Conditions

`if`, `elif`, and `else` use block syntax. There is no `then` keyword.

```wyrm
if x > 1 {
    print("greater")
} elif x == 1 {
    print("equal")
} else {
    print("less")
}
```

## Loops

The current loop form is `do / til`. `repeat` is accepted as an alias for `do`.

```wyrm
i = 0
do {
    print(i)
    i = i + 1
} til (i >= 5)
```

## Arrays

Arrays support literals, indexing, slicing, and assignment by index.

```wyrm
numbers = [10, 20, 30]
numbers[0] = 99
print(numbers[0])
print(len(numbers))
```

## Modules

Modules are imported with `use`.

```wyrm
use helper.wyr;

fn main() {
    print("module loaded")
}
```

`use` statements require a trailing semicolon.

## Printing And Input

`print` accepts multiple arguments.

```wyrm
name = input("Enter name: ")
print("Hello,", name)
```

## Ownership And Memory

The language includes ownership, RAII, arenas, and unsafe blocks.

```wyrm
owned items = [1, 2, 3]
next = items
```

`unsafe` blocks are required for raw memory operations.

```wyrm
unsafe {
    ptr = malloc(64)
    ptr = realloc(ptr, 128)
    free(ptr)
}
```

Arena allocation allows scope-bound block allocations:

```wyrm
arena buf(256)
p = buf.alloc(64)
```

## String & Data Library

Wyrm provides 14 built-in operations for string and data manipulation:

- `split(s, sep)`: Splits string `s` by separator `sep`, returning an array of strings.
- `join(sep, lst)`: Joins array of strings `lst` with separator `sep`, returning a string.
- `trim(s)`: Strips leading and trailing whitespace from string `s`.
- `upper(s)`: Converts string `s` to uppercase.
- `lower(s)`: Converts string `s` to lowercase.
- `contains(s, sub)`: Returns `true` if `sub` is a substring of `s`.
- `replace(s, old, new)`: Replaces all occurrences of `old` with `new` in string `s`.
- `starts_with(s, prefix)`: Returns `true` if string `s` starts with `prefix`.
- `ends_with(s, suffix)`: Returns `true` if string `s` ends with `suffix`.
- `char_at(s, i)`: Returns character at index `i` of string `s`.
- `ord_val(c)`: Returns the numeric character code of single-character string `c`.
- `chr_val(n)`: Returns a single-character string from the character code `n`.
- `to_bytes(s)`: Converts string `s` to an array of numeric byte values.
- `from_bytes(lst)`: Reconstructs a string from an array of numeric byte values `lst`.

## Native Build Flow

Build the toolchain from source using the self-contained C11 bootstrap compiler:

```bash
gcc wyrm/scr/bootstrap.c -o bootstrap.exe -std=c11 -O2
./bootstrap.exe
```

Run and compile programs:

```bash
wyrmc run examples/hello.wyr
wyrmc build examples/hello.wyr
```

Check tool versions:

```bash
wyrmc --version
wyrpkg --version
```

## GitHub Language Stats

The intended repository language order is Wyrm first, C second, and minor tooling languages after that. `.gitattributes` excludes Python, browser assets, generated output, and cache metadata from language stats. GitHub can display Wyrm only after the language is registered in GitHub Linguist.
